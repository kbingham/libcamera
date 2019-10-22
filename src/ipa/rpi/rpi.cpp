/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Raspberry Pi Ltd.
 * Copyright (C) 2019, Google Inc.
 *
 * rpi.cpp - Raspberry Pi Image Processing Algorithms
 */

#include <algorithm>
#include <cstdint>
#include <math.h>
#include <queue>
#include <string.h>

#include <linux/bcm2835_isp.h>

#include <ipa/ipa_interface.h>
#include <ipa/ipa_module_info.h>
#include <ipa/raspberrypi.h>
#include <libcamera/buffer.h>
#include <libcamera/control_ids.h>
#include <libcamera/request.h>

#include "log.h"
#include "utils.h"

#include "../libipa/ipa_interface_wrapper.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(IPARPI)

/* Only facilitating a dummy initial example... */
enum rpi_ae_state {
	RPI_AE_NOLOCK,
	/* Other states? */
	RPI_AE_LOCKED,
};

class IPARPi : public IPAInterface
{
public:
	int init() override { return 0; }

	void configure(const std::map<unsigned int, IPAStream> &streamConfig,
		       const std::map<unsigned int, ControlInfoMap> &entityControls) override;
	void mapBuffers(const std::vector<IPABuffer> &buffers) override;
	void unmapBuffers(const std::vector<unsigned int> &ids) override;
	void processEvent(const IPAOperationData &event) override;

private:
	void queueRequest(unsigned int frame, rpi_isp_params_cfg *params,
			  const ControlList &controls);
	void updateStatistics(unsigned int frame,
			      const rpi_stat_buffer *stats);

	void setControls(unsigned int frame);
	void metadataReady(unsigned int frame, unsigned int aeState);

	std::map<unsigned int, BufferMemory> bufferInfo_;

	ControlInfoMap ctrls_;

	/* Camera sensor controls. */
	bool autoExposure_;
	uint32_t exposure_;
	uint32_t minExposure_;
	uint32_t maxExposure_;
	uint32_t gain_;
	uint32_t minGain_;
	uint32_t maxGain_;
};

void IPARPi::configure(const std::map<unsigned int, IPAStream> &streamConfig,
		       const std::map<unsigned int, ControlInfoMap> &entityControls)
{
	if (entityControls.empty())
		return;

	ctrls_ = entityControls.at(0);

	const auto itExp = ctrls_.find(V4L2_CID_EXPOSURE);
	if (itExp == ctrls_.end()) {
		LOG(IPARPI, Error) << "Can't find exposure control";
		return;
	}

	const auto itGain = ctrls_.find(V4L2_CID_ANALOGUE_GAIN);
	if (itGain == ctrls_.end()) {
		LOG(IPARPI, Error) << "Can't find gain control";
		return;
	}

	autoExposure_ = true;

	minExposure_ = std::max<uint32_t>(itExp->second.min().get<int32_t>(), 1);
	maxExposure_ = itExp->second.max().get<int32_t>();
	exposure_ = minExposure_;

	minGain_ = std::max<uint32_t>(itGain->second.min().get<int32_t>(), 1);
	maxGain_ = itGain->second.max().get<int32_t>();
	gain_ = minGain_;

	LOG(IPARPI, Info)
		<< "Exposure: " << minExposure_ << "-" << maxExposure_
		<< " Gain: " << minGain_ << "-" << maxGain_;

	setControls(0);
}

void IPARPi::mapBuffers(const std::vector<IPABuffer> &buffers)
{
	for (const IPABuffer &buffer : buffers) {
		bufferInfo_[buffer.id] = buffer.memory;
		bufferInfo_[buffer.id].planes()[0].mem();
	}
}

void IPARPi::unmapBuffers(const std::vector<unsigned int> &ids)
{
	for (unsigned int id : ids)
		bufferInfo_.erase(id);
}

void IPARPi::processEvent(const IPAOperationData &event)
{
	switch (event.operation) {
	case RPI_IPA_EVENT_SIGNAL_STAT_BUFFER: {
		unsigned int frame = event.data[0];
		unsigned int bufferId = event.data[1];

		const rpi_stat_buffer *stats =
			static_cast<rpi_stat_buffer *>(bufferInfo_[bufferId].planes()[0].mem());

		updateStatistics(frame, stats);
		break;
	}
	case RPI_IPA_EVENT_QUEUE_REQUEST: {
		unsigned int frame = event.data[0];
		unsigned int bufferId = event.data[1];

		rpi_isp_params_cfg *params =
			static_cast<rpi_isp_params_cfg *>(bufferInfo_[bufferId].planes()[0].mem());

		queueRequest(frame, params, event.controls[0]);
		break;
	}
	default:
		LOG(IPARPI, Error) << "Unknown event " << event.operation;
		break;
	}
}

void IPARPi::queueRequest(unsigned int frame, rpi_isp_params_cfg *params,
			  const ControlList &controls)
{
	/* Prepare parameters buffer. */
	memset(params, 0, sizeof(*params));

	/* Auto Exposure on/off. */
	if (controls.contains(controls::AeEnable)) {
		autoExposure_ = controls.get(controls::AeEnable);
		if (autoExposure_) {
			/* Update configuration ? */
		}
	}

	IPAOperationData op;
	op.operation = RPI_IPA_ACTION_PARAM_FILLED;

	queueFrameAction.emit(frame, op);
}

void IPARPi::updateStatistics(unsigned int frame,
			      const rpi_stat_buffer *stats)
{
	enum rpi_ae_state aeState = RPI_AE_NOLOCK;

	/* Do anything here */

	metadataReady(frame, aeState);
}

void IPARPi::setControls(unsigned int frame)
{
	IPAOperationData op;
	op.operation = RPI_IPA_ACTION_V4L2_SET;

	ControlList ctrls(ctrls_);
	ctrls.set(V4L2_CID_EXPOSURE, static_cast<int32_t>(exposure_));
	ctrls.set(V4L2_CID_ANALOGUE_GAIN, static_cast<int32_t>(gain_));
	op.controls.push_back(ctrls);

	queueFrameAction.emit(frame, op);
}

void IPARPi::metadataReady(unsigned int frame, unsigned int aeState)
{
	ControlList ctrls(controls::controls);

	if (aeState)
		ctrls.set(controls::AeLocked, aeState == RPI_AE_LOCKED);

	IPAOperationData op;
	op.operation = RPI_IPA_ACTION_METADATA;
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
	"PipelineHandlerRPi",
	"RPi IPA",
	"LGPL-2.1-or-later",
};

struct ipa_context *ipaCreate()
{
	return new IPAInterfaceWrapper(new IPARPi());
}
}; /* extern "C" */

}; /* namespace libcamera */
