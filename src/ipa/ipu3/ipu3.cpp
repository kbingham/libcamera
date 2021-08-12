/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * ipu3.cpp - IPU3 Image Processing Algorithms
 */

#include <stdint.h>

#include <linux/intel-ipu3.h>
#include <linux/v4l2-controls.h>

#include <libcamera/base/log.h>

#include <libcamera/control_ids.h>
#include <libcamera/framebuffer.h>
#include <libcamera/request.h>

#include <libcamera/ipa/ipa_interface.h>
#include <libcamera/ipa/ipa_module_info.h>
#include <libcamera/ipa/ipu3_ipa_interface.h>

#include "libcamera/internal/mapped_framebuffer.h"

#include "libipa/camera_sensor_helper.h"

#include "algorithms/algorithm.h"
#include "algorithms/grid.h"
#include "ipu3_agc.h"
#include "ipu3_awb.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(IPAIPU3)

namespace ipa::ipu3 {

class IPAIPU3 : public IPAIPU3Interface
{
public:
	int init(const IPASettings &settings) override;
	int start() override;
	void stop() override {}

	int configure(const IPAConfigInfo &configInfo) override;

	void mapBuffers(const std::vector<IPABuffer> &buffers) override;
	void unmapBuffers(const std::vector<unsigned int> &ids) override;
	void processEvent(const IPU3Event &event) override;

private:
	void processControls(unsigned int frame, const ControlList &controls);
	void fillParams(unsigned int frame, ipu3_uapi_params *params);
	void parseStatistics(unsigned int frame,
			     int64_t frameTimestamp,
			     const ipu3_uapi_stats_3a *stats);

	void setControls(unsigned int frame);
	void calculateBdsGrid(const Size &bdsOutputSize);

	std::map<unsigned int, MappedFrameBuffer> buffers_;

	ControlInfoMap ctrls_;

	IPACameraSensorInfo sensorInfo_;

	/* Camera sensor controls. */
	uint32_t defVBlank_;
	uint32_t exposure_;
	uint32_t minExposure_;
	uint32_t maxExposure_;
	uint32_t gain_;
	uint32_t minGain_;
	uint32_t maxGain_;

	/* Interface to the AWB algorithm */
	std::unique_ptr<IPU3Awb> awbAlgo_;
	/* Interface to the AEC/AGC algorithm */
	std::unique_ptr<IPU3Agc> agcAlgo_;
	/* Interface to the Camera Helper */
	std::unique_ptr<CameraSensorHelper> camHelper_;

	/* Maintain the algorithms used by the IPA */
	std::list<std::unique_ptr<ipa::ipu3::Algorithm>> algorithms_;

	/* Local parameter storage */
	struct IPAContext context_;
	struct ipu3_uapi_params params_;
};

int IPAIPU3::init(const IPASettings &settings)
{
	camHelper_ = CameraSensorHelperFactory::create(settings.sensorModel);
	if (camHelper_ == nullptr) {
		LOG(IPAIPU3, Error) << "Failed to create camera sensor helper for " << settings.sensorModel;
		return -ENODEV;
	}

	/* Construct our Algorithms */
	algorithms_.emplace_back(new algorithms::Grid());

	return 0;
}

int IPAIPU3::start()
{
	setControls(0);

	return 0;
}

int IPAIPU3::configure(const IPAConfigInfo &configInfo)
{
	if (configInfo.entityControls.empty()) {
		LOG(IPAIPU3, Error) << "No controls provided";
		return -ENODATA;
	}

	sensorInfo_ = configInfo.sensorInfo;

	ctrls_ = configInfo.entityControls.at(0);

	const auto itExp = ctrls_.find(V4L2_CID_EXPOSURE);
	if (itExp == ctrls_.end()) {
		LOG(IPAIPU3, Error) << "Can't find exposure control";
		return -EINVAL;
	}

	const auto itGain = ctrls_.find(V4L2_CID_ANALOGUE_GAIN);
	if (itGain == ctrls_.end()) {
		LOG(IPAIPU3, Error) << "Can't find gain control";
		return -EINVAL;
	}

	const auto itVBlank = ctrls_.find(V4L2_CID_VBLANK);
	if (itVBlank == ctrls_.end()) {
		LOG(IPAIPU3, Error) << "Can't find VBLANK control";
		return -EINVAL;
	}

	minExposure_ = std::max(itExp->second.min().get<int32_t>(), 1);
	maxExposure_ = itExp->second.max().get<int32_t>();
	exposure_ = minExposure_;

	minGain_ = std::max(itGain->second.min().get<int32_t>(), 1);
	maxGain_ = itGain->second.max().get<int32_t>();
	gain_ = minGain_;

	defVBlank_ = itVBlank->second.def().get<int32_t>();

	/* Clean context and IPU3 parameters at configuration */
	params_ = {};
	context_ = {};

	for (auto const &algo : algorithms_) {
		int ret = algo->configure(context_, configInfo);
		if (ret)
			return ret;
	}

	awbAlgo_ = std::make_unique<IPU3Awb>();
	awbAlgo_->initialise(params_, context_.configuration.grid.bdsOutputSize, context_.configuration.grid.bdsGrid);

	agcAlgo_ = std::make_unique<IPU3Agc>();
	agcAlgo_->initialise(context_.configuration.grid.bdsGrid, sensorInfo_);

	return 0;
}

void IPAIPU3::mapBuffers(const std::vector<IPABuffer> &buffers)
{
	for (const IPABuffer &buffer : buffers) {
		const FrameBuffer fb(buffer.planes);
		buffers_.emplace(buffer.id,
				 MappedFrameBuffer(&fb, MappedFrameBuffer::MapFlag::ReadWrite));
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

		parseStatistics(event.frame, event.frameTimestamp, stats);
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
	for (auto const &algo : algorithms_)
		algo->prepare(context_, params_);

	if (agcAlgo_->updateControls())
		awbAlgo_->updateWbParameters(params_, agcAlgo_->gamma());

	*params = params_;

	IPU3Action op;
	op.op = ActionParamFilled;

	queueFrameAction.emit(frame, op);
}

void IPAIPU3::parseStatistics(unsigned int frame,
			      [[maybe_unused]] int64_t frameTimestamp,
			      [[maybe_unused]] const ipu3_uapi_stats_3a *stats)
{
	ControlList ctrls(controls::controls);

	double gain = camHelper_->gain(gain_);
	agcAlgo_->process(stats, exposure_, gain);
	gain_ = camHelper_->gainCode(gain);

	awbAlgo_->calculateWBGains(stats);

	if (agcAlgo_->updateControls())
		setControls(frame);

	/* \todo Use VBlank value calculated from each frame exposure. */
	int64_t frameDuration = sensorInfo_.lineLength * (defVBlank_ + sensorInfo_.outputSize.height) /
				(sensorInfo_.pixelRate / 1e6);
	ctrls.set(controls::FrameDuration, frameDuration);

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

} /* namespace ipa::ipu3 */

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
