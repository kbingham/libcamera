/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2017 Intel Corporation.
 *
 * ParameterEncoder.cpp: Encode AIC parameters to IPU3 kernel interface
 *
 * This implementation is highly derived from ChromeOS:
 *   platform2/camera/hal/intel/ipu3/psl/ipu3/workers/IPU3AicToFwEncoder.cpp
 */

#include "parameter_encoder.h"

#include <assert.h>

namespace libcamera {

/* Auto White Balance */
#define AWB_FR_MAX_GRID_CELLS_IN_ONE_SET 32
#define AWB_FR_GRID_DIM_MASK 0x3F

static void ispAwbFrEncode(aic_config *config, ipu3_uapi_params *params)
{
	unsigned int coeff_sum = 0, nf_val = 1;

	params->acc_param.awb_fr.bayer_coeff[0] = config->awb_fr_2500_config.awb_fr.bayer_coeffs.A1;
	params->acc_param.awb_fr.bayer_coeff[1] = config->awb_fr_2500_config.awb_fr.bayer_coeffs.A2;
	params->acc_param.awb_fr.bayer_coeff[2] = config->awb_fr_2500_config.awb_fr.bayer_coeffs.A3;
	params->acc_param.awb_fr.bayer_coeff[3] = config->awb_fr_2500_config.awb_fr.bayer_coeffs.A4;
	params->acc_param.awb_fr.bayer_coeff[4] = config->awb_fr_2500_config.awb_fr.bayer_coeffs.A5;
	params->acc_param.awb_fr.bayer_coeff[5] = config->awb_fr_2500_config.awb_fr.bayer_coeffs.A6;
	params->acc_param.awb_fr.bayer_sign = config->awb_fr_2500_config.awb_fr.bayer_coeffs.sign_vec;

	coeff_sum += config->awb_fr_2500_config.awb_fr.bayer_coeffs.A1;
	coeff_sum += config->awb_fr_2500_config.awb_fr.bayer_coeffs.A2;
	coeff_sum += config->awb_fr_2500_config.awb_fr.bayer_coeffs.A3;
	coeff_sum += config->awb_fr_2500_config.awb_fr.bayer_coeffs.A4;
	coeff_sum += config->awb_fr_2500_config.awb_fr.bayer_coeffs.A5;
	coeff_sum += config->awb_fr_2500_config.awb_fr.bayer_coeffs.A6;

	while (coeff_sum >> nf_val) {
		++nf_val;
	}

	--nf_val;

	if (nf_val < 7 || nf_val > 12)
		params->acc_param.awb_fr.bayer_nf = 7;
	else
		params->acc_param.awb_fr.bayer_nf = nf_val;

	params->acc_param.awb_fr.grid_cfg.width = config->awb_fr_2500_config.awb_fr.grid.grid_width & AWB_FR_GRID_DIM_MASK;
	params->acc_param.awb_fr.grid_cfg.height = config->awb_fr_2500_config.awb_fr.grid.grid_height & AWB_FR_GRID_DIM_MASK;
	params->acc_param.awb_fr.grid_cfg.block_width_log2 = config->awb_fr_2500_config.awb_fr.grid.block_width & 0x7;
	params->acc_param.awb_fr.grid_cfg.block_height_log2 = config->awb_fr_2500_config.awb_fr.grid.block_height & 0x7;

	assert(config->awb_fr_2500_config.awb_fr.grid.grid_width);
	params->acc_param.awb_fr.grid_cfg.height_per_slice =
		(unsigned char)(AWB_FR_MAX_GRID_CELLS_IN_ONE_SET / config->awb_fr_2500_config.awb_fr.grid.grid_width);
	params->acc_param.awb_fr.grid_cfg.x_start = config->awb_fr_2500_config.awb_fr.grid.x_start & 0xFFF;
	params->acc_param.awb_fr.grid_cfg.y_start = (config->awb_fr_2500_config.awb_fr.grid.y_start & 0xFFF) | IPU3_UAPI_GRID_Y_START_EN;

	params->use.acc_awb_fr = 1;
}

void ParameterEncoder::encode(aic_config *config, ipu3_uapi_params *params)
{
	/*
	 * Do not encode parameters until the algorithms are run,
	 * or assertions will be hit
	 */
	return;

	ispAwbFrEncode(config, params);

	return;
}

} /* namespace libcamera */
