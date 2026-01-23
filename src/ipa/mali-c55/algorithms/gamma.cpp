/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2026, Ideas On Board
 *
 * MaliC55 RGB Gamma control
 */

#include "gamma.h"

#include <algorithm>

#include <libipa/fixedpoint.h>

namespace libcamera {

namespace ipa::mali_c55::algorithms {

/**
 * \class RGBGamma
 * \brief MaliC55 Gamma out correction
 *
 * This algorithm implements the output gamma correction using the libipa
 * GammaAlgorithm class.
 *
 * The hardware uses 129 evenly spaced points for its lookup table, and
 * applies linear interpolation between them.
 *
 * The same module is also capable of applying an offset and gain to each channel.
 * Currently a fixed gain of 1, and a fixed offset of 0 is used.
 * \todo Implement global contrast adjustment using offsets and gains.
 */

/**
 * \copydoc libcamera::ipa::Algorithm::init
 */
int RGBGamma::init(IPAContext &context, const ValueNode &tuningData)
{
	return gamma_.init(context.ctrlMap, tuningData);
}

/**
 * \copydoc libcamera::ipa::Algorithm::configure
 */
int RGBGamma::configure(IPAContext &context,
			[[maybe_unused]] const IPACameraSensorInfo &configInfo)
{
	gamma_.configure(context.activeState.gamma);

	return 0;
}

/**
 * \copydoc libcamera::ipa::Algorithm::queueRequest
 */
void RGBGamma::queueRequest(IPAContext &context, const uint32_t frame,
			    IPAFrameContext &frameContext,
			    const ControlList &controls)
{
	gamma_.queueRequest(context.activeState.gamma, frame, frameContext.gamma, controls);
}

/**
 * \copydoc libcamera::ipa::Algorithm::prepare
 */
void RGBGamma::prepare([[maybe_unused]] IPAContext &context,
		       [[maybe_unused]] const uint32_t frame,
		       IPAFrameContext &frameContext,
		       MaliC55Params *params)
{
	if (!frameContext.gamma.update)
		return;

	auto frConfig = params->block<MaliC55Blocks::GammaFr>();
	frConfig.setEnabled(true);

	gamma_.prepare(frameContext.gamma, std::span(frConfig->lut));

	std::fill(frConfig->gains, std::end(frConfig->gains), UQ<4, 8>(1.0f).quantized());
	std::fill(frConfig->offs, std::end(frConfig->offs), UQ<0, 12>(0.0f).quantized());

	/* Enable the same gamma correction on the "downscale" pipeline as well. */
	auto dsConfig = params->block<MaliC55Blocks::GammaDs>();
	dsConfig.setEnabled(true);

	std::copy(frConfig->gains, std::end(frConfig->gains), dsConfig->gains);
	std::copy(frConfig->offs, std::end(frConfig->offs), dsConfig->offs);
	std::copy(frConfig->lut, std::end(frConfig->lut), dsConfig->lut);
}

/**
 * \copydoc libcamera::ipa::Algorithm::process
 */
void RGBGamma::process([[maybe_unused]] IPAContext &context,
		       [[maybe_unused]] const uint32_t frame,
		       IPAFrameContext &frameContext,
		       [[maybe_unused]] const mali_c55_stats_buffer *stats,
		       ControlList &metadata)
{
	gamma_.process(frameContext.gamma, metadata);
}

REGISTER_IPA_ALGORITHM(RGBGamma, "Gamma")

} /* namespace ipa::mali_c55::algorithms */

} /* namespace libcamera */
