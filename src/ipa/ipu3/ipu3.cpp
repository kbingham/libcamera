/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * ipu3.cpp - IPU3 Image Processing Algorithms
 */

#include <stdint.h>
#include <sys/mman.h>

#include <linux/intel-ipu3.h>
#include <linux/v4l2-controls.h>

#include <libcamera/buffer.h>
#include <libcamera/control_ids.h>
#include <libcamera/ipa/ipa_interface.h>
#include <libcamera/ipa/ipa_module_info.h>
#include <libcamera/ipa/ipu3_ipa_interface.h>
#include <libcamera/request.h>

#include "libcamera/internal/buffer.h"
#include "libcamera/internal/log.h"

/* IA AIQ Wrapper API */
#include "aic/aic.h"
#include "aiq/aiq.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(IPAIPU3)

namespace ipa {

namespace ipu3 {

class IPAIPU3 : public IPAIPU3Interface
{
public:
	int init(const IPASettings &settings) override;

	int start() override;
	void stop() override {}

	void configure(const CameraSensorInfo &sensorInfo,
		       const std::map<uint32_t, ControlInfoMap> &entityControls,
		       const Size &bdsOutputSize, const Size &ifSize,
		       const Size &gdcSize, const Size &cropRegion) override;

	void mapBuffers(const std::vector<IPABuffer> &buffers) override;
	void unmapBuffers(const std::vector<unsigned int> &ids) override;
	void processEvent(const IPU3Event &event) override;

private:
	void processControls(unsigned int frame, const ControlList &controls);
	void fillParams(unsigned int frame, ipu3_uapi_params *params);
	void parseStatistics(unsigned int frame,
			     const ipu3_uapi_stats_3a *stats);

	void setControls(unsigned int frame);

	std::map<unsigned int, MappedFrameBuffer> buffers_;

	ControlInfoMap ctrls_;

	/* Camera sensor controls. */
	uint32_t exposure_;
	uint32_t minExposure_;
	uint32_t maxExposure_;
	uint32_t gain_;
	uint32_t minGain_;
	uint32_t maxGain_;

	/* Intel Library Instances. */
	aiq::AIQ aiq_;
	aic::AIC aic_;

	/* Temporary storage until we have a FrameContext object / struct */
	aiq::AiqInputParameters aiqInputParams_;
	aiq::AiqResults results_;
};

int IPAIPU3::init([[maybe_unused]] const IPASettings &settings)
{
	int ret;

	/* Temporary mapping of the sensor name to the AIQB data file. */
	std::map<std::string, std::string> aiqb_paths = {
		{ "ov13858", "/usr/share/libcamera/ipa/ipu3/00ov13858.aiqb" },
		{ "ov5670", "/usr/share/libcamera/ipa/ipu3/01ov5670.aiqb" },
		{ "ov5693", "/usr/share/libcamera/ipa/ipu3/01ov5670.aiqb" }, /// NOT THE RIGHT TUNING FILE
		{ "imx258", "/etc/camera/ipu3/00imx258.aiqb" },
	};

	LOG(IPAIPU3, Info) << "Initialising IPA IPU3 for "
			   << settings.sensorModel;

	auto it = aiqb_paths.find(settings.sensorModel);
	if (it != aiqb_paths.end())
		LOG(IPAIPU3, Info) << "Using tuning file: " << it->second;

	ret = aiq_.init();
	if (ret)
		return ret;

	aiqInputParams_.init();

	return aic_.init();
}

int IPAIPU3::start()
{
	setControls(0);

	return 0;
}

void IPAIPU3::configure(const CameraSensorInfo &sensorInfo,
			const std::map<uint32_t, ControlInfoMap> &entityControls,
			const Size &bdsOutputSize,
			const Size &ifSize,
			const Size &gdcSize,
			const Size &cropRegion)
{
	if (entityControls.empty())
		return;

	ctrls_ = entityControls.at(0);

	const auto itExp = ctrls_.find(V4L2_CID_EXPOSURE);
	if (itExp == ctrls_.end()) {
		LOG(IPAIPU3, Error) << "Can't find exposure control";
		return;
	}

	const auto itGain = ctrls_.find(V4L2_CID_ANALOGUE_GAIN);
	if (itGain == ctrls_.end()) {
		LOG(IPAIPU3, Error) << "Can't find gain control";
		return;
	}

	minExposure_ = std::max(itExp->second.min().get<int32_t>(), 1);
	maxExposure_ = itExp->second.max().get<int32_t>();
	exposure_ = maxExposure_;

	minGain_ = std::max(itGain->second.min().get<int32_t>(), 1);
	maxGain_ = itGain->second.max().get<int32_t>();
	gain_ = maxGain_;

	if (aiq_.configure()) {
		LOG(IPAIPU3, Error) << "Failed to configure the AIQ";
		return;
	}

	if (aiqInputParams_.configureSensorParams(sensorInfo)) {
		LOG(IPAIPU3, Error) << "Failed to configure AiqInputParams";
		return;
	}

	if (aic_.configure(bdsOutputSize, ifSize, gdcSize, cropRegion)) {
		LOG(IPAIPU3, Error) << "Failed to configure the AIC";
		return;
	}

	/* Set AE/AWB defaults, this typically might not belong here */
	aiqInputParams_.setAeAwbAfDefaults();
}

void IPAIPU3::mapBuffers(const std::vector<IPABuffer> &buffers)
{
	for (const IPABuffer &buffer : buffers) {
		const FrameBuffer fb(buffer.planes);
		buffers_.emplace(buffer.id,
				 MappedFrameBuffer(&fb, PROT_READ | PROT_WRITE));
	}
}

void IPAIPU3::unmapBuffers(const std::vector<unsigned int> &ids)
{
	for (unsigned int id : ids) {
		auto it = buffers_.find(id);
		if (it == buffers_.end())
			continue;

		buffers_.erase(it);
	}
}

void IPAIPU3::processEvent(const IPU3Event &event)
{
	switch (event.op) {
	case EventProcessControls: {
		processControls(event.frame, event.controls);
		break;
	}
	case EventStatReady: {
		auto it = buffers_.find(event.bufferId);
		if (it == buffers_.end()) {
			LOG(IPAIPU3, Error) << "Could not find stats buffer!";
			return;
		}

		Span<uint8_t> mem = it->second.maps()[0];
		const ipu3_uapi_stats_3a *stats =
			reinterpret_cast<ipu3_uapi_stats_3a *>(mem.data());

		parseStatistics(event.frame, stats);
		break;
	}
	case EventFillParams: {
		auto it = buffers_.find(event.bufferId);
		if (it == buffers_.end()) {
			LOG(IPAIPU3, Error) << "Could not find param buffer!";
			return;
		}

		Span<uint8_t> mem = it->second.maps()[0];
		ipu3_uapi_params *params =
			reinterpret_cast<ipu3_uapi_params *>(mem.data());

		fillParams(event.frame, params);
		break;
	}
	default:
		LOG(IPAIPU3, Error) << "Unknown event " << event.op;
		break;
	}
}

void IPAIPU3::processControls([[maybe_unused]] unsigned int frame,
			      [[maybe_unused]] const ControlList &controls)
{
	/* \todo Start processing for 'frame' based on 'controls'. */
}

void IPAIPU3::fillParams(unsigned int frame, ipu3_uapi_params *params)
{
	/* Prepare parameters buffer. */
	memset(params, 0, sizeof(*params));

	/*
	 * Call into the AIQ object, and set up the library with any requested
	 * controls or settings from the incoming request.
	 *
	 * (statistics are fed into the library as a separate event
	 *  when available)
	 *
	 * - Run algorithms
	 *
	 * - Fill params buffer with the results of the algorithms.
	 */

	/* Run algorithms into/using this context structure */
	if (frame % 4 == 0)
		aiq_.run2a(frame, aiqInputParams_, results_);

	aic_.updateRuntimeParams(results_);
	aic_.run(params);

	/*
	 * We expect this to be moved later, and perhaps algorithsm will be run
	 * when the statistics come in rather than when the params are filled...
	 */
	aiq::dumpExposure(results_.ae()->exposures);

	exposure_ = results_.ae()->exposures[0].sensor_exposure->coarse_integration_time;
	gain_ = results_.ae()->exposures[0].sensor_exposure->analog_gain_code_global;
	setControls(frame);

	IPU3Action op;
	op.op = ActionParamFilled;

	queueFrameAction.emit(frame, op);
}

void IPAIPU3::parseStatistics(unsigned int frame,
			      [[maybe_unused]] const ipu3_uapi_stats_3a *stats)
{
	ControlList ctrls(controls::controls);

	/* \todo React to statistics and update internal state machine. */
	/* \todo Add meta-data information to ctrls. */

	/* *stats comes from the IPU3 hardware. We need to give this data into
	 * the AIQ library
	 */

	/* todo:  We need to have map at least the timestamp of the buffer
	 * of the statistics in to allow the library to identify how long
	 * convergence takes. Without it = the algos will not converge. */

	aiq_.setStatistics(frame, results_, stats);

	IPU3Action op;
	op.op = ActionMetadataReady;
	op.controls = ctrls;

	queueFrameAction.emit(frame, op);
}

void IPAIPU3::setControls(unsigned int frame)
{
	IPU3Action op;
	op.op = ActionSetSensorControls;

	ControlList ctrls(ctrls_);
	ctrls.set(V4L2_CID_EXPOSURE, static_cast<int32_t>(exposure_));
	ctrls.set(V4L2_CID_ANALOGUE_GAIN, static_cast<int32_t>(gain_));
	op.controls = ctrls;

	queueFrameAction.emit(frame, op);
}

} /* namespace ipu3 */

} /* namespace ipa */

/*
 * External IPA module interface
 */

extern "C" {
const struct IPAModuleInfo ipaModuleInfo = {
	IPA_MODULE_API_VERSION,
	1,
	"PipelineHandlerIPU3",
	"ipu3",
};

IPAInterface *ipaCreate()
{
	return new ipa::ipu3::IPAIPU3();
}
}

} /* namespace libcamera */
