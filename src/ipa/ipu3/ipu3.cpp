/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * ipu3.cpp - IPU3 Image Processing Algorithms
 */

#include <algorithm>
#include <math.h>
#include <queue>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>

#include <linux/intel-ipu3.h>
#include <linux/v4l2-controls.h>

#include <libcamera/buffer.h>
#include <libcamera/control_ids.h>
#include <libcamera/ipa/ipa_interface.h>
#include <libcamera/ipa/ipa_module_info.h>
#include <libcamera/ipa/ipu3.h>
#include <libcamera/request.h>

#include <libipa/ipa_interface_wrapper.h>

#include "libcamera/internal/log.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(IPAIPU3)

class IPAIPU3 : public IPAInterface
{
public:
	int init([[maybe_unused]] const IPASettings &settings) override
	{
		return 0;
	}
	int start() override { return 0; }
	void stop() override {}

	void configure(const CameraSensorInfo &info,
		       const std::map<unsigned int, IPAStream> &streamConfig,
		       const std::map<unsigned int, const ControlInfoMap &> &entityControls,
		       const IPAOperationData &ipaConfig,
		       IPAOperationData *response) override;
	void mapBuffers(const std::vector<IPABuffer> &buffers) override;
	void unmapBuffers(const std::vector<unsigned int> &ids) override;
	void processEvent(const IPAOperationData &event) override;

private:
	void fillParams(unsigned int frame, ipu3_uapi_params *params,
			const ControlList &controls);

	void parseStatistics(unsigned int frame,
			     const ipu3_uapi_stats_3a *stats);

	void setControls(unsigned int frame);

	std::map<unsigned int, FrameBuffer> buffers_;
	std::map<unsigned int, void *> buffersMemory_;

	ControlInfoMap ctrls_;

	/* Camera sensor controls. */
	uint32_t exposure_;
	uint32_t minExposure_;
	uint32_t maxExposure_;
	uint32_t gain_;
	uint32_t minGain_;
	uint32_t maxGain_;
};

void IPAIPU3::configure([[maybe_unused]] const CameraSensorInfo &info,
			[[maybe_unused]] const std::map<unsigned int, IPAStream> &streamConfig,
			const std::map<unsigned int, const ControlInfoMap &> &entityControls,
			[[maybe_unused]] const IPAOperationData &ipaConfig,
			[[maybe_unused]] IPAOperationData *result)
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

	minExposure_ = std::max<uint32_t>(itExp->second.min().get<int32_t>(), 1);
	maxExposure_ = itExp->second.max().get<int32_t>();
	exposure_ = maxExposure_;

	minGain_ = std::max<uint32_t>(itGain->second.min().get<int32_t>(), 1);
	maxGain_ = itGain->second.max().get<int32_t>();
	gain_ = maxGain_;

	setControls(0);
}

void IPAIPU3::mapBuffers(const std::vector<IPABuffer> &buffers)
{
	for (const IPABuffer &buffer : buffers) {
		auto elem = buffers_.emplace(std::piecewise_construct,
					     std::forward_as_tuple(buffer.id),
					     std::forward_as_tuple(buffer.planes));
		const FrameBuffer &fb = elem.first->second;

		buffersMemory_[buffer.id] = mmap(NULL,
						 fb.planes()[0].length,
						 PROT_READ | PROT_WRITE,
						 MAP_SHARED,
						 fb.planes()[0].fd.fd(),
						 0);

		if (buffersMemory_[buffer.id] == MAP_FAILED) {
			int ret = -errno;
			LOG(IPAIPU3, Fatal) << "Failed to mmap buffer: "
					    << strerror(-ret);
		}
	}
}

void IPAIPU3::unmapBuffers(const std::vector<unsigned int> &ids)
{
	for (unsigned int id : ids) {
		const auto fb = buffers_.find(id);
		if (fb == buffers_.end())
			continue;

		munmap(buffersMemory_[id], fb->second.planes()[0].length);
		buffersMemory_.erase(id);
		buffers_.erase(id);
	}
}

void IPAIPU3::processEvent(const IPAOperationData &event)
{
	switch (event.operation) {
	case IPU3_IPA_EVENT_PARSE_STAT: {
		unsigned int frame = event.data[0];
		unsigned int bufferId = event.data[1];

		const ipu3_uapi_stats_3a *stats =
			static_cast<ipu3_uapi_stats_3a *>(buffersMemory_[bufferId]);

		parseStatistics(frame, stats);
		break;
	}
	case IPU3_IPA_EVENT_FILL_PARAMS: {
		unsigned int frame = event.data[0];
		unsigned int bufferId = event.data[1];

		ipu3_uapi_params *params =
			static_cast<ipu3_uapi_params *>(buffersMemory_[bufferId]);

		fillParams(frame, params, event.controls[0]);
		break;
	}
	default:
		LOG(IPAIPU3, Error) << "Unknown event " << event.operation;
		break;
	}
}

void IPAIPU3::fillParams(unsigned int frame, ipu3_uapi_params *params,
			 [[maybe_unused]] const ControlList &controls)
{
	/* Prepare parameters buffer. */
	memset(params, 0, sizeof(*params));

	/* \todo Fill in parameters buffer. */

	IPAOperationData op;
	op.operation = IPU3_IPA_ACTION_PARAM_FILLED;

	queueFrameAction.emit(frame, op);

	/* \todo Calculate new values for exposure_ and gain_. */
	setControls(frame);
}

void IPAIPU3::parseStatistics(unsigned int frame,
			      [[maybe_unused]] const ipu3_uapi_stats_3a *stats)
{
	ControlList ctrls(controls::controls);

	/* \todo React to statistics and update internal state machine. */
	/* \todo Add meta-data information to ctrls. */

	IPAOperationData op;
	op.operation = IPU3_IPA_ACTION_METADATA_READY;
	op.controls.push_back(ctrls);

	queueFrameAction.emit(frame, op);
}

void IPAIPU3::setControls(unsigned int frame)
{
	IPAOperationData op;
	op.operation = IPU3_IPA_ACTION_SET_SENSOR_CONTROLS;

	ControlList ctrls(ctrls_);
	ctrls.set(V4L2_CID_EXPOSURE, static_cast<int32_t>(exposure_));
	ctrls.set(V4L2_CID_ANALOGUE_GAIN, static_cast<int32_t>(gain_));
	op.controls.push_back(ctrls);

	queueFrameAction.emit(frame, op);
}

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

struct ipa_context *ipaCreate()
{
	return new IPAInterfaceWrapper(std::make_unique<IPAIPU3>());
}
}

} /* namespace libcamera */
