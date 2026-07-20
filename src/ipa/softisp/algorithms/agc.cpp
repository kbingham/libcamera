/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Red Hat Inc.
 *
 * Exposure and gain
 */

#include "agc.h"

#include <libcamera/base/log.h>

#include <libipa/histogram.h>

#include "control_ids.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(IPASoftIspExposure)

namespace ipa::softisp::algorithms {

int Agc::configure(IPAContext &context, [[maybe_unused]] const IPAConfigInfo &configInfo)
{
	agc_.setLimits({
		.exposure = {
			context.configuration.agc.exposureMin,
			context.configuration.agc.exposureMax,
		},
		.gain = {
			context.configuration.agc.againMin,
			context.configuration.agc.againMax,
		},
		.gainMinStep = context.configuration.agc.againMinStep,
		.gain1 = context.configuration.agc.again10,
	});

	return 0;
}

void Agc::process(IPAContext &context,
		  [[maybe_unused]] const uint32_t frame,
		  IPAFrameContext &frameContext,
		  const SwIspStats *stats,
		  ControlList &metadata)
{
	utils::Duration exposureTime =
		context.configuration.agc.lineDuration * frameContext.sensor.exposure;
	metadata.set(controls::ExposureTime, exposureTime.get<std::micro>());
	metadata.set(controls::AnalogueGain, frameContext.sensor.gain);

	if (!context.activeState.agc.valid) {
		/*
		 * Init active-state from sensor values in case updateExposure()
		 * does not run for the first frame.
		 */
		context.activeState.agc.exposure = frameContext.sensor.exposure;
		context.activeState.agc.again = frameContext.sensor.gain;
		context.activeState.agc.valid = true;
	}

	if (!stats->valid) {
		/*
		 * Use the new exposure and gain values calculated the last time
		 * there were valid stats.
		 */
		frameContext.agc.exposure = context.activeState.agc.exposure;
		frameContext.agc.gain = context.activeState.agc.again;
		return;
	}

	/* \todo The histogram should come already adjusted. */
	auto histogram = stats->yHistogram;
	const unsigned int blackLevelHistIdx =
		context.activeState.blc.level * histogram.size() / 256;

	for (unsigned int i = 0; i < blackLevelHistIdx; i++)
		histogram[blackLevelHistIdx] += histogram[i];

	const auto &newEv = agc_.calculateNewEv({
		.yHist = {
			{ histogram.begin() + blackLevelHistIdx, histogram.end() },
		},
		.exposure = frameContext.sensor.exposure,
		.gain = frameContext.sensor.gain,
	});

	frameContext.agc.exposure = newEv.exposure;
	frameContext.agc.gain = newEv.analogueGain;

	context.activeState.agc.exposure = frameContext.agc.exposure;
	context.activeState.agc.again = frameContext.agc.gain;
}

REGISTER_IPA_ALGORITHM(Agc, "Agc")

} /* namespace ipa::softisp::algorithms */

} /* namespace libcamera */
