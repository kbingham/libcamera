/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2026 Ideas on Board Oy
 *
 * Auto exposure/gain algorithm for implementing the IPA-specific AGC algorithms
 */

#pragma once

#include <stdint.h>
#include <utility>

#include <linux/v4l2-controls.h>

#include <libcamera/controls.h>

#include "camera_sensor_helper.h"

namespace libcamera {

namespace ipa {

namespace agc {

[[nodiscard]]
inline std::pair<uint32_t, double>
extractControls(const ControlList &controls, const CameraSensorHelper *sensor)
{
	auto exposure = controls.get(V4L2_CID_EXPOSURE).get<int32_t>();
	auto gainCode = controls.get(V4L2_CID_ANALOGUE_GAIN).get<int32_t>();

	return {
		static_cast<uint32_t>(exposure),
		sensor ? sensor->gain(gainCode) : gainCode,
	};
}

inline void
prepareControls(ControlList &controls, const CameraSensorHelper *sensor,
		uint32_t exposure, double gain)
{
	controls.set(V4L2_CID_EXPOSURE, static_cast<int32_t>(exposure));
	controls.set(V4L2_CID_ANALOGUE_GAIN,
		     static_cast<int32_t>(sensor
					  ? sensor->gainCode(gain)
					  : static_cast<uint32_t>(gain)));
}

} /* namespace agc */

} /* namespace ipa */

} /* namespace libcamera */
