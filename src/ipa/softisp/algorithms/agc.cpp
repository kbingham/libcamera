/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Red Hat Inc.
 *
 * Exposure and gain
 */

#include "agc.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <stdint.h>

#include <libcamera/base/log.h>

#include <libipa/histogram.h>

#include "control_ids.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(IPASoftIspExposure)

namespace ipa::softisp::algorithms {

/*
 * The number of bins to use for the optimal exposure calculations.
 */
static constexpr unsigned int kExposureBinsCount = 5;

/*
 * The exposure is optimal when the mean sample value of the histogram is
 * in the middle of the range.
 */
static constexpr float kExposureOptimal = kExposureBinsCount / 2.0;

/*
 * This implements the hysteresis for the exposure adjustment.
 * It is small enough to have the exposure close to the optimal, and is big
 * enough to prevent the exposure from wobbling around the optimal value.
 */
static constexpr float kExposureSatisfactory = 0.2;

/*
 * Proportional gain for exposure/gain adjustment. Maps the MSV error to a
 * multiplicative correction factor:
 *
 *   factor = 1.0 + kExpProportionalGain * error
 *
 * With kExpProportionalGain = 0.04:
 *   - max error ~2.5 -> factor 1.10 (~10% step, same as before)
 *   - error 1.0      -> factor 1.04 (~4% step)
 *   - error 0.3      -> factor 1.012 (~1.2% step)
 *
 * This replaces the fixed 10% bang-bang step with a proportional correction
 * that converges smoothly and avoids overshooting near the target.
 */
static constexpr float kExpProportionalGain = 0.04;

/*
 * Maximum multiplicative step per frame, to bound the correction when the
 * scene changes dramatically.
 */
static constexpr float kExpMaxStep = 0.15;

namespace {

std::optional<float> calculateMSV(const Histogram &histogram)
{
	/*
	 * Calculate Mean Sample Value (MSV) according to formula from:
	 * https://www.araa.asn.au/acra/acra2007/papers/paper84final.pdf
	 */
	const unsigned int yHistValsPerBin = histogram.bins() / kExposureBinsCount;
	const unsigned int yHistValsPerBinMod =
		histogram.bins() / (histogram.bins() % kExposureBinsCount + 1);
	int exposureBins[kExposureBinsCount] = {};
	unsigned int denom = 0;
	unsigned int num = 0;

	if (yHistValsPerBin == 0)
		return std::nullopt;

	for (unsigned int i = 0; i < histogram.bins(); i++) {
		unsigned int idx = (i - (i / yHistValsPerBinMod)) / yHistValsPerBin;
		exposureBins[idx] += histogram[i];
	}

	for (unsigned int i = 0; i < kExposureBinsCount; i++) {
		LOG(IPASoftIspExposure, Debug) << i << ": " << exposureBins[i];
		denom += exposureBins[i];
		num += exposureBins[i] * (i + 1);
	}

	return (denom == 0 ? 0 : static_cast<float>(num) / denom);
}

} /* namespace */

Agc::Agc()
{
}

void Agc::updateExposure(IPAContext &context, IPAFrameContext &frameContext, double exposureMSV)
{
	uint32_t exposure = frameContext.sensor.exposure;
	double again = frameContext.sensor.gain;

	double error = kExposureOptimal - exposureMSV;

	if (std::abs(error) <= kExposureSatisfactory)
		return;

	/*
	 * Compute a proportional correction factor. The sign of the error
	 * determines the direction: positive error means too dark (increase),
	 * negative means too bright (decrease).
	 */
	float step = std::clamp(static_cast<float>(error) * kExpProportionalGain,
				-kExpMaxStep, kExpMaxStep);
	float factor = 1.0f + step;

	if (factor > 1.0f) {
		/* Scene too dark: increase exposure first, then gain. */
		if (exposure < context.configuration.agc.exposureMax) {
			uint32_t next = exposure * factor;
			exposure = std::max(next, exposure + 1);
		} else {
			double next = again * factor;
			again = std::max(next, again + context.configuration.agc.againMinStep);
		}
	} else {
		/* Scene too bright: decrease gain first, then exposure. */
		if (again > context.configuration.agc.again10) {
			double next = again * factor;
			again = std::min(next, again - context.configuration.agc.againMinStep);
		} else {
			uint32_t next = exposure * factor;
			exposure = std::min(next, exposure - 1);
		}
	}

	exposure = std::clamp(exposure, context.configuration.agc.exposureMin,
			      context.configuration.agc.exposureMax);
	again = std::clamp(again, context.configuration.agc.againMin,
			   context.configuration.agc.againMax);

	frameContext.agc.exposure = exposure;
	frameContext.agc.gain = again;

	context.activeState.agc.exposure = exposure;
	context.activeState.agc.again = again;

	LOG(IPASoftIspExposure, Debug)
		<< "exposureMSV " << exposureMSV
		<< " error " << error << " factor " << factor
		<< " exp " << exposure << " again " << again;
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

	auto exposureMSV = calculateMSV({ { histogram.begin() + blackLevelHistIdx, histogram.end() } });
	if (!exposureMSV) {
		LOG(IPASoftIspExposure, Debug)
			<< "Not adjusting exposure due to insufficient histogram data";
		return;
	}

	updateExposure(context, frameContext, *exposureMSV);
}

REGISTER_IPA_ALGORITHM(Agc, "Agc")

} /* namespace ipa::softisp::algorithms */

} /* namespace libcamera */
