/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * aiq.cpp - Intel IA Imaging library C++ wrapper
 */

#include "aiq.h"

#include <libcamera/class.h>

#include "libcamera/internal/file.h"
#include "libcamera/internal/log.h"

#include "binary_data.h"
#include "stats/ipa_ipu3_stats.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(AIQ)

namespace ipa::ipu3::aiq {

AIQ::AIQ()
	: aiq_(nullptr)
{
	LOG(AIQ, Info) << "Creating IA AIQ Wrapper";
}

AIQ::~AIQ()
{
	LOG(AIQ, Info) << "Destroying IA AIQ Wrapper";
	if (aiq_)
		ia_aiq_deinit(aiq_);

	if (iaCmc_)
		ia_cmc_parser_deinit(iaCmc_);

	delete aiqStats_;
	aiqStats_ = nullptr;
}

std::string AIQ::decodeError(ia_err err)
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

int AIQ::init(BinaryData &aiqb, BinaryData &nvm, BinaryData &aiqd)
{
	constexpr unsigned int maxGridW = 80;
	constexpr unsigned int maxGridH = 60;
	constexpr unsigned int maxExposures = 1;

	/* \todo: No maker note provided. */
	ia_mkn *ia_mkn = nullptr;

	/*
	 * \todo: Both the AIC and the AIQ use the iaCmc_.
	 * Can this be the same instance or do they need their own instances?
	 */
	iaCmc_ = ia_cmc_parser_init(aiqb.data());

	aiq_ = ia_aiq_init(aiqb.data(), nvm.data(), aiqd.data(),
			   maxGridW, maxGridH, maxExposures,
			   iaCmc_, ia_mkn);
	if (!aiq_) {
		LOG(AIQ, Error) << "Failed to initialise aiq library";
		return -ENODATA;
	}

	version_ = ia_aiq_get_version();
	LOG(AIQ, Info) << "AIQ Library version: " << version_;

	aiqStats_ = new IPAIPU3Stats;

	return 0;
}

int AIQ::configure()
{
	LOG(AIQ, Debug) << "Configure AIQ";

	return 0;
}

int AIQ::setStatistics(unsigned int frame,
		       int64_t timestamp,
		       AiqResults &results,
		       const ipu3_uapi_stats_3a *stats)
{
	ia_aiq_statistics_input_params *statParams =
		aiqStats_->getInputStatsParams(frame, &results, stats);

	statParams->frame_timestamp = timestamp;

	ia_err err = ia_aiq_statistics_set(aiq_, statParams);
	if (err) {
		LOG(AIQ, Error) << "Failed to set statistics: "
				<< decodeError(err);
		return -ENODATA;
	}

	return 0;
}

/*
 * Run algorithms, and store the configuration in the parameters buffers
 * This is likely to change drastically as we progress, and the algorithms
 * might run asycnronously, or after receipt of statistics, with the filling
 * of the parameter buffer being the only part handled when called for.
 */
int AIQ::run(unsigned int frame, AiqInputParameters &params,
	     AiqResults &results)
{
	(void)frame;
	(void)params;

	/* Run AWB algorithms, using the config structures. */
	afRun(params.afParams, results);
	afBracketRun(params.afBracketParams, results);
	gbceRun(params.gbceParams, results);
	aeRun(params.aeInputParams, results);
	awbRun(params.awbParams, results);
	dsdRun(params.dsdParams, results);
	parameterAdapterRun(params.paParams, results);
	/* \todo: shadingAdapterRun(parms.saParams, results):
	 * blocking on curating ia_aiq_frame_params input param. */

	return 0;
}

int AIQ::run2a(unsigned int frame, aiq::AiqInputParameters &params,
	       aiq::AiqResults &results)
{
	(void)frame;

	/* Run AWB algorithms, using the config structures. */
	aeRun(params.aeInputParams, results);
	awbRun(params.awbParams, results);

	params.paParams.awb_results = results.awb();
	params.paParams.exposure_params = results.ae()->exposures[0].exposure;
	parameterAdapterRun(params.paParams, results);

	gbceRun(params.gbceParams, results);
	afRun(params.afParams, results);

	return 0;
}

int AIQ::afRun(ia_aiq_af_input_params &afParams, AiqResults &results)
{
	ia_aiq_af_results *afResults = nullptr;

	ia_err err = ia_aiq_af_run(aiq_, &afParams, &afResults);
	if (err) {
		LOG(AIQ, Error) << "Failed to run Auto-focus: "
				<< decodeError(err);
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

		results.setAf(afResults);
	} else {
		LOG(AIQ, Error) << "Auto Focus produced no results";
	}

	return 0;
}

int AIQ::afBracketRun(ia_aiq_af_bracket_input_params &afBracketParams,
		      AiqResults &results)
{
	ia_aiq_af_bracket_results *afBracketResults = nullptr;
	ia_err err = ia_aiq_af_bracket(aiq_, &afBracketParams, &afBracketResults);
	if (err) {
		LOG(AIQ, Error) << "Failed to run AF Bracket: "
				<< decodeError(err);
		return err;
	}

	if (afBracketResults) {
		LOG(AIQ, Debug) << "=== AF Bracket ==="
				<< "distances_bracketing: " << *afBracketResults->distances_bracketing << "\n"
				<< "lens_positions_bracketing: " << *afBracketResults->lens_positions_bracketing << "\n";

		results.setAfBracket(afBracketResults);
	} else {
		LOG(AIQ, Error) << "AFBracket produced no results";
	}

	return 0;
}

/* Global Brightness and Contrast Enhancement */
int AIQ::gbceRun(ia_aiq_gbce_input_params &gbceParams, AiqResults &results)
{
	ia_aiq_gbce_results *gbceResults = nullptr;

	ia_err err = ia_aiq_gbce_run(aiq_, &gbceParams, &gbceResults);
	if (err) {
		LOG(AIQ, Error) << "Failed to run GBCE: "
				<< decodeError(err);
		return err;
	}

	if (gbceResults) {
		LOG(AIQ, Info) << "GBCE: GammaLutSize: " << gbceResults->gamma_lut_size
			       << " ToneMap Size: " << gbceResults->tone_map_lut_size;

		results.setGbce(gbceResults);
	} else {
		LOG(AIQ, Error) << "GBCE produced no results";
	}

	return 0;
}

int AIQ::aeRun(ia_aiq_ae_input_params &aeParams, AiqResults &results)
{
	ia_aiq_ae_results *aeResults = nullptr;
	ia_err err = ia_aiq_ae_run(aiq_, &aeParams, &aeResults);
	if (err) {
		LOG(AIQ, Error) << "Failed to run AutoExposure: "
				<< decodeError(err);
		return err;
	}

	if (aeResults) {
		LOG(AIQ, Info) << "AE Coarse:" << aeResults->exposures->sensor_exposure->coarse_integration_time
			       << " AE LLP: " << aeResults->exposures->sensor_exposure->line_length_pixels
			       << " AE FLL: " << aeResults->exposures->sensor_exposure->frame_length_lines;

		results.setAe(aeResults);
	} else {
		LOG(AIQ, Error) << "AE: No results";
	}

	return 0;
}

int AIQ::awbRun(ia_aiq_awb_input_params &awbParams, AiqResults &results)
{
	/* Todo: Determine if this is required, or can be a nullptr */
	ia_aiq_awb_results awb_result_alloc = {};
	ia_aiq_awb_results *awbResults = &awb_result_alloc;
	ia_err err = ia_aiq_awb_run(aiq_, &awbParams, &awbResults);
	if (err) {
		LOG(AIQ, Error) << "Failed to run Auto-white-balance: "
				<< decodeError(err);
		return err;
	}

	if (awbResults) {
		LOG(AIQ, Info) << "Final R/G: " << awbResults->final_r_per_g << "\n"
			       << "Final B/G: " << awbResults->final_b_per_g << "\n"
			       << "ConvergenceDistance. : " << awbResults->distance_from_convergence;

		results.setAwb(awbResults);
	} else {
		LOG(AIQ, Error) << "No AWB results...";
	}

	return 0;
}

int AIQ::dsdRun(ia_aiq_dsd_input_params &dsdParams, AiqResults &results)
{
	ia_aiq_scene_mode detectedSceneMode;
	ia_err err = ia_aiq_dsd_run(aiq_, &dsdParams, &detectedSceneMode);
	if (err) {
		LOG(AIQ, Error) << "Failed to run Determine Scene detection: "
				<< decodeError(err);
		return err;
	}

	results.setDetectedSceneMode(detectedSceneMode);

	return 0;
}

int AIQ::parameterAdapterRun(ia_aiq_pa_input_params &paParams,
			     AiqResults &results)
{
	ia_aiq_pa_results *paResults = nullptr;
	ia_err err = ia_aiq_pa_run(aiq_, &paParams, &paResults);
	if (err) {
		LOG(AIQ, Error) << "Failed to run parameter adapter: "
				<< decodeError(err);
		return err;
	}

	if (paResults) {
		LOG(AIQ, Debug) << "Parameter Adapter brightness level"
				<< paResults->brightness_level << "\n";

		results.setPa(paResults);
	} else {
		LOG(AIQ, Error) << "No Parameter Adapater results...";
	}

	return 0;
}

int AIQ::shadingAdapterRun(ia_aiq_sa_input_params &saParams,
			   AiqResults &results)
{
	ia_aiq_sa_results *saResults = nullptr;
	ia_err err = ia_aiq_sa_run(aiq_, &saParams, &saResults);
	if (err) {
		LOG(AIQ, Error) << "Failed to run shading adapter: "
				<< decodeError(err);
		return err;
	}

	if (saResults) {
		LOG(AIQ, Debug) << "LSC width: " << saResults->width
				<< " LSC height: " << saResults->height
				<< " LSC updated: "
				<< (saResults->lsc_update ? "True" : "False");

		results.setSa(saResults);
	} else {
		LOG(AIQ, Error) << "No Shading Adapater results...";
	}

	return 0;
}

/******************************************************************************/
LOG_DEFINE_CATEGORY(AIQDEBUG)

void dumpExposure(ia_aiq_ae_exposure_result *exp)
{
	ia_aiq_exposure_parameters *exposure = exp->exposure;
	//ia_aiq_exposure_sensor_parameters *sensor = exp->sensor_exposure;

	LOG(AIQDEBUG, Info) << "Exposure Parameters: Index " << exp->exposure_index;
	LOG(AIQDEBUG, Info) << " Distance from Convergence: "
			    << exp->distance_from_convergence << (exp->converged ? " " : " NOT ") << "Converged";

	LOG(AIQDEBUG, Info) << " T:" << exposure->exposure_time_us
			    << " AG:" << exposure->analog_gain
			    << " DG: " << exposure->digital_gain
			    << " Apt:" << exposure->aperture_fn
			    << " Tgt:" << exposure->total_target_exposure
			    << " ISO: " << exposure->iso;

	LOG(AIQDEBUG, Info) << " Coarse - " << exp->sensor_exposure->coarse_integration_time;
}

} /* namespace ipa::ipu3::aiq */

} /* namespace libcamera */
