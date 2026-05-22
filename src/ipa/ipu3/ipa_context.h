/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * IPU3 IPA Context
 *
 */

#pragma once

#include <linux/intel-ipu3.h>

#include <libcamera/base/utils.h>

#include <libcamera/controls.h>
#include <libcamera/geometry.h>

#include <libcamera/ipa/core_ipa_interface.h>

#include <libipa/agc.h>
#include <libipa/camera_sensor_helper.h>
#include <libipa/fc_queue.h>

namespace libcamera {

namespace ipa::ipu3 {

struct IPASessionConfiguration {
	struct {
		ipu3_uapi_grid_config bdsGrid;
		Size bdsOutputSize;
		uint32_t stride;
	} grid;

	struct {
		ipu3_uapi_grid_config afGrid;
	} af;

	agc::Session agc;
};

struct IPAActiveState {
	struct {
		uint32_t focus;
		double maxVariance;
		bool stable;
	} af;

	agc::ActiveState agc;

	struct {
		struct {
			double red;
			double green;
			double blue;
		} gains;

		double temperatureK;
	} awb;

	struct {
		double gamma;
		struct ipu3_uapi_gamma_corr_lut gammaCorrection;
	} toneMapping;
};

struct IPAFrameContext : public FrameContext {
	struct {
		uint32_t exposure;
		double gain;
	} sensor;

	agc::FrameContext agc;
};

struct IPAContext {
	IPAContext(unsigned int frameContextSize)
		: frameContexts(frameContextSize)
	{
	}

	IPASessionConfiguration configuration;
	IPACameraSensorInfo sensorInfo;
	ControlInfoMap sensorControls;
	IPAActiveState activeState;

	FCQueue<IPAFrameContext> frameContexts;

	std::unique_ptr<CameraSensorHelper> camHelper;

	ControlInfoMap::Map ctrlMap;
};

} /* namespace ipa::ipu3 */

} /* namespace libcamera*/
