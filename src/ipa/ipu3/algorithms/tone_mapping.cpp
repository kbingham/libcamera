/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google inc.
 *
 * tone_mapping.cpp - IPU3 ToneMapping and Gamma control
 */

#include "tone_mapping.h"

#include <cmath>
#include <string.h>

/**
 * \file tone_mapping.h
 */

namespace libcamera {

namespace ipa::ipu3::algorithms {

/**
 * \class ToneMapping
 * \brief A class to handle tone mapping based on gamma
 *
 * This algorithm improves the image dynamic using a look-up table which is
 * generated based on a gamma parameter.
 *
 * Gamma values less than one have the effect of compressing the image histogram
 * while values over 1 will expand it.
 *
 * Expanding the histogram has the effect of providing better overall contrast.
 */

ToneMapping::ToneMapping()
	: gamma_(1.0)
{
}

/**
 * \brief Fill in the parameter structure, and enable gamma control
 * \param context The shared IPA context
 * \param params The IPU3 parameters
 *
 * Populate the IPU3 parameter structure with our gamma correction table, and
 * enable the gamma control module in the accelerator cluster.
 */
void ToneMapping::prepare([[maybe_unused]] IPAContext &context,
			  ipu3_uapi_params *params)
{
	/* Copy the calculated LUT into the parameters buffer. */
	memcpy(params->acc_param.gamma.gc_lut.lut,
	       context.frameContext.toneMapping.gammaCorrection.lut,
	       IPU3_UAPI_GAMMA_CORR_LUT_ENTRIES *
	       sizeof(params->acc_param.gamma.gc_lut.lut[0]));

	/* Enable the custom gamma table. */
	params->use.acc_gamma = 1;
	params->acc_param.gamma.gc_ctrl.enable = 1;
}

/**
 * \brief Calculate the Gamma curve
 * \param context The shared IPA context
 * \param stats The IPU3 statistics and ISP results
 *
 * The gamma correction look up table is generated as an inverse power curve
 * from our gamma setting.
 */
void ToneMapping::process(IPAContext &context,
			  [[maybe_unused]] const ipu3_uapi_stats_3a *stats)
{
	/*
	 * Hardcode gamma to 1.1 as a default for now.
	 *
	 * \todo Expose gamma control setting through the libcamera control API
	 */
	gamma_ = 1.1;

	if (context.frameContext.toneMapping.gamma == gamma_)
		return;

	struct ipu3_uapi_gamma_corr_lut &lut =
		context.frameContext.toneMapping.gammaCorrection;

	for (uint32_t i = 0; i < std::size(lut.lut); i++) {
		double j = static_cast<double>(i) / (std::size(lut.lut) - 1);
		double gamma = std::pow(j, 1.0 / gamma_);

		/* The output value is expressed on 13 bits. */
		lut.lut[i] = gamma * 8191;
	}

	context.frameContext.toneMapping.gamma = gamma_;
}

} /* namespace ipa::ipu3::algorithms */

} /* namespace libcamera */
