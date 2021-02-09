/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * aiq.cpp - Intel IA Imaging library C++ wrapper
 */

#include "libcamera/internal/file.h"
#include "libcamera/internal/log.h"

#include "parameter_encoder.h"

#include "aiq.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(AIQ)

static std::string ia_err_decode(ia_err err)
{
	static const char *errors[] = {
		"None", /*!< No errors*/
		"General", /*!< General error*/
		"Memory", /*!< Out of memory*/
		"Corrupted", /*!< Corrupted data*/
		"Internal", /*!< Error in code*/
		"Invalid", /*!< Invalid argument for a function*/
		"Disabled", /*!< Functionality is disabled*/
	};

	std::ostringstream o;

	o << "[";

	unsigned int count = 0;
	for (unsigned int i = 0; i <= 6; i++) {
		if (err & (1 << i)) {
			if (count)
				o << ",";

			o << errors[i];
			count++;
		}
	}

	o << "]";

	return o.str();
}

/**
 * \brief Binary Data wrapper
 *
 * Loads data from a file, and returns it as an ia_binary_data type.
 * Data is freed automatically when the object goes out of scope.
 */
class AIQBinaryData
{
public:
	// Disallow copy / assign...
	AIQBinaryData();

	int load(const char *filename);
	ia_binary_data *data() { return &iaBinaryData_; }

private:
	ia_binary_data iaBinaryData_;
	std::vector<uint8_t> data_;
};

AIQBinaryData::AIQBinaryData()
{
	iaBinaryData_.data = nullptr;
	iaBinaryData_.size = 0;
}

int AIQBinaryData::load(const char *filename)
{
	File binary(filename);

	if (!binary.exists()) {
		LOG(AIQ, Error) << "Failed to find file: " << filename;
		return -ENOENT;
	}

	if (!binary.open(File::ReadOnly)) {
		LOG(AIQ, Error) << "Failed to open: " << filename;
		return -EINVAL;
	}

	ssize_t fileSize = binary.size();
	if (fileSize < 0) {
		LOG(AIQ, Error) << "Failed to determine fileSize: " << filename;
		return -ENODATA;
	}

	data_.resize(fileSize);

	int bytesRead = binary.read(data_);
	if (bytesRead != fileSize) {
		LOG(AIQ, Error) << "Failed to read file: " << filename;
		return -EINVAL;
	}

	iaBinaryData_.data = data_.data();
	iaBinaryData_.size = fileSize;

	LOG(AIQ, Info) << "Successfully loaded: " << filename;

	return 0;
}

AIQ::AIQ()
{
	LOG(AIQ, Info) << "Creating IA AIQ Wrapper";
}

AIQ::~AIQ()
{
	LOG(AIQ, Info) << "Destroying IA AIQ Wrapper";
	ia_aiq_deinit(aiq_);
}

int AIQ::init()
{
	AIQBinaryData aiqb;
	AIQBinaryData nvm;
	AIQBinaryData aiqd;

	unsigned int stats_max_width = 1920;
	unsigned int stats_max_height = 1080;
	unsigned int max_num_stats_in = 4;
	ia_cmc_t *ia_cmc = nullptr;
	ia_mkn *ia_mkn = nullptr;

	int ret = aiqb.load("/usr/share/libcamera/ipa/ipu3/01ov5670.aiqb");
	if (ret) {
		LOG(AIQ, Error) << "Not quitting";
	}

	/* Width, Height, other parameters to be set as parameters? */
	aiq_ = ia_aiq_init(aiqb.data(), nvm.data(), aiqd.data(),
			   stats_max_width, stats_max_height, max_num_stats_in,
			   ia_cmc, ia_mkn);
	if (!aiq_) {
		LOG(AIQ, Error) << "Failed to initialise aiq library";
		return -ENODATA;
	}

	version_ = ia_aiq_get_version();
	LOG(AIQ, Info) << "AIQ Library version: " << version_;

	return 0;
}

int AIQ::configure()
{
	LOG(AIQ, Debug) << "Configure AIQ";

	return 0;
}

int AIQ::setStatistics(unsigned int frame, const ipu3_uapi_stats_3a *stats)
{
	LOG(AIQ, Debug) << "Set Statistitcs";

	(void)frame;
	(void)stats;
	ia_aiq_statistics_input_params stats_param = {};

	/* We should give the converted statistics into the AIQ library here. */

	ia_err err = ia_aiq_statistics_set(aiq_, &stats_param);
	if (err) {
		LOG(AIQ, Error) << "Failed to set statistics: "
				<< ia_err_decode(err);

		LOG(AIQ, Error) << "Not quitting";
	}

	return 0;
}

/*
 * Run algorithms, and store the configuration in the parameters buffers
 * This is likely to change drastically as we progress, and the algorithms
 * might run asycnronously, or after receipt of statistics, with the filling
 * of the parameter buffer being the only part handled when called for.
 */
int AIQ::run(unsigned int frame, ipu3_uapi_params *params)
{
	(void)frame;

	/* Run AWB algorithms, using the config structures. */

	af_run();
	gbce_run();
	ae_run();
	awb_run();

	/* IPU3 firmware specific encoding for ISP controls. */
	ParameterEncoder::encode(&config_, params);

	return 0;
}

int AIQ::af_run()
{
	ia_aiq_af_input_params afParams = {
		ia_aiq_frame_use_still, 0, 1500,
		ia_aiq_af_operation_mode_auto,
		ia_aiq_af_range_normal,
		ia_aiq_af_metering_mode_auto,
		ia_aiq_flash_mode_auto,
		NULL, NULL, false
	}; // Get from input params

	ia_aiq_af_results *afResults = nullptr;

	ia_err err = ia_aiq_af_run(aiq_, &afParams, &afResults);
	if (err) {
		LOG(AIQ, Error) << "Failed to run Auto-focus: "
				<< ia_err_decode(err);
		return err;
	}

	if (afResults) {
		LOG(AIQ, Info) << "AF: Focal distance " << afResults->current_focus_distance;
		LOG(AIQ, Debug) << "=== AUTO FOCUS ==="
			       << "AutoFocus status: " << afResults->status << "\n"
			       << "Focal distance: " << afResults->current_focus_distance << "\n"
			       << "next_lens_position: " << afResults->next_lens_position << "\n"
			       << "lens_driver_action: " << afResults->lens_driver_action << "\n"
			       << "use_af_assist: " << afResults->use_af_assist << "\n"
			       << "Final lens pos: " << afResults->final_lens_position_reached << "\n\n";
	} else {
		LOG(AIQ, Error) << "Auto Focus produced no results";
	}

	ia_aiq_af_bracket_input_params af_bracket_params = {
		2,
		*afResults,
		ia_aiq_af_bracket_mode_symmetric
	};
	ia_aiq_af_bracket_results *af_bracket_results = nullptr;
	err = ia_aiq_af_bracket(aiq_, &af_bracket_params, &af_bracket_results);
	if (err) {
		LOG(AIQ, Error) << "Failed to run Auto-focus Bracketting: "
				<< ia_err_decode(err);
		return err;
	}

	if (af_bracket_results) {
		LOG(AIQ, Debug) << "=== AUTO FOCUS BRACKETTING ==="
				<< "distances_bracketing: " << *af_bracket_results->distances_bracketing << "\n"
				<< "lens_positions_bracketing: " << *af_bracket_results->lens_positions_bracketing;
	} else {
		LOG(AIQ, Error) << "Auto Focus Bracketing produced no results";
	}

	ia_aiq_dsd_input_params dsd_params{
		afResults,
		ia_aiq_scene_mode_landscape
	};

	ia_aiq_scene_mode detected_scene_mode;
	err = ia_aiq_dsd_run(aiq_, &dsd_params, &detected_scene_mode);
	if (err) {
		LOG(AIQ, Error) << "Failed to run Detect Scene: "
				<< ia_err_decode(err);
		return err;
	}

	LOG(AIQ, Info) << "DSD: Detected: " << detected_scene_mode;

	/* TODO: Parse and set afResults somewhere */

	return 0;
}

int AIQ::gbce_run()
{
	ia_aiq_gbce_input_params params = {
		ia_aiq_gbce_level_use_tuning,
		ia_aiq_tone_map_level_use_tuning,
		ia_aiq_frame_use_still,
		0
	}; // TODO: Set/store externally
	ia_aiq_gbce_results *gbceResults = nullptr;

	ia_err err = ia_aiq_gbce_run(aiq_, &params, &gbceResults);
	if (err) {
		LOG(AIQ, Error) << "Failed to run GBCE: "
				<< ia_err_decode(err);
		return err;
	}

	return 0;
}

int AIQ::ae_run()
{
	ia_aiq_ae_input_params ae_params = {
		2, /* Multiple exposure for exposure bracketing. */
		ia_aiq_frame_use_still, ia_aiq_flash_mode_auto,
		ia_aiq_ae_operation_mode_automatic,
		ia_aiq_ae_metering_mode_evaluative,
		ia_aiq_ae_priority_mode_normal,
		ia_aiq_ae_flicker_reduction_auto,
		NULL, /* AE descriptor sensor? param, mandatory */
		NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, -1,
		ia_aiq_aperture_control_dc_iris_auto,
		ia_aiq_ae_exposure_distribution_auto, -1
	};

	ia_aiq_ae_results *aeResults = nullptr;
	ia_err err = ia_aiq_ae_run(aiq_, &ae_params, &aeResults);
	if (err) {
		LOG(AIQ, Error) << "Failed to run AutoExposure: "
				<< ia_err_decode(err);
		return err;
	}

	if (aeResults) {
		LOG(AIQ, Info) << "AE:"
			       << " Num:" << aeResults->num_exposures
			       << " lux: " << aeResults->lux_level_estimate
			       << " F:" << aeResults->aperture_control->aperture_fn;
	} else {
		LOG(AIQ, Error) << "AE: No results";
	}

	return 0;
}

int AIQ::awb_run()
{
	ia_aiq_awb_input_params awbParams = {
		ia_aiq_frame_use_still, ia_aiq_awb_operation_mode_auto,
		nullptr, nullptr, 0.0
	};

	ia_aiq_awb_results awb_result_alloc = {};
	ia_aiq_awb_results *awbResults = &awb_result_alloc;
	ia_err err = ia_aiq_awb_run(aiq_, &awbParams, &awbResults);
	if (err) {
		LOG(AIQ, Error) << "Failed to run Auto-white-balance: "
				<< ia_err_decode(err);
		return err;
	}

	if (awbResults) {
		LOG(AIQ, Info) << "Final R/G: " << awbResults->final_r_per_g << "\n"
			       << "Final B/G: " << awbResults->final_b_per_g << "\n"
			       << "ConvergenceDistance. : " << awbResults->distance_from_convergence;
	} else {
		LOG(AIQ, Error) << "No AWB results...";
	}

	return 0;
}

} /* namespace libcamera */
