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

#include <algorithm>
#include <assert.h>
#include <string.h>

namespace libcamera {

/* Auto White Balance */
#define AWB_FR_MAX_GRID_CELLS_IN_ONE_SET 32
#define AWB_FR_GRID_DIM_MASK 0x3F

/* Auto Exposure */
#define AE_NUM_OF_WEIGHT_ELEMS 96

/* Imported directly from CommonUtilMacros.h */
#ifndef MEMCPY_S
#define MEMCPY_S(dest, dmax, src, smax) memcpy((dest), (src), std::min((size_t)(dmax), (size_t)(smax)))
#endif

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

static void ispAeEncode(aic_config *config, ipu3_uapi_params *params)
{
	params->acc_param.ae.grid_cfg.ae_en = 1;
	params->acc_param.ae.grid_cfg.block_height_log2 = config->ae_2500_config.ae.ae_grid_config.block_height;
	params->acc_param.ae.grid_cfg.block_width_log2 = config->ae_2500_config.ae.ae_grid_config.block_width;
	params->acc_param.ae.grid_cfg.width = config->ae_2500_config.ae.ae_grid_config.grid_width;
	params->acc_param.ae.grid_cfg.height = config->ae_2500_config.ae.ae_grid_config.grid_height;
	params->acc_param.ae.grid_cfg.x_start = config->ae_2500_config.ae.ae_grid_config.x_start;
	params->acc_param.ae.grid_cfg.y_start = config->ae_2500_config.ae.ae_grid_config.y_start;

	for (int elem_index = 0; elem_index < AE_NUM_OF_WEIGHT_ELEMS; elem_index++) {
		params->acc_param.ae.weights[elem_index].cell0 = config->ae_2500_config.ae.ae_weights.val[8 * elem_index];
		params->acc_param.ae.weights[elem_index].cell1 = config->ae_2500_config.ae.ae_weights.val[8 * elem_index + 1];
		params->acc_param.ae.weights[elem_index].cell2 = config->ae_2500_config.ae.ae_weights.val[8 * elem_index + 2];
		params->acc_param.ae.weights[elem_index].cell3 = config->ae_2500_config.ae.ae_weights.val[8 * elem_index + 3];
		params->acc_param.ae.weights[elem_index].cell4 = config->ae_2500_config.ae.ae_weights.val[8 * elem_index + 4];
		params->acc_param.ae.weights[elem_index].cell5 = config->ae_2500_config.ae.ae_weights.val[8 * elem_index + 5];
		params->acc_param.ae.weights[elem_index].cell6 = config->ae_2500_config.ae.ae_weights.val[8 * elem_index + 6];
		params->acc_param.ae.weights[elem_index].cell7 = config->ae_2500_config.ae.ae_weights.val[8 * elem_index + 7];
	}

	params->acc_param.ae.ae_ccm.gain_gr = config->ae_2500_config.ae.ae_ccm.wb_coeffs.gain_GR;
	params->acc_param.ae.ae_ccm.gain_r = config->ae_2500_config.ae.ae_ccm.wb_coeffs.gain_R;
	params->acc_param.ae.ae_ccm.gain_b = config->ae_2500_config.ae.ae_ccm.wb_coeffs.gain_B;
	params->acc_param.ae.ae_ccm.gain_gb = config->ae_2500_config.ae.ae_ccm.wb_coeffs.gain_GB;

	MEMCPY_S(params->acc_param.ae.ae_ccm.mat, sizeof(params->acc_param.ae.ae_ccm.mat),
		 config->ae_2500_config.ae.ae_ccm.mat_coeffs.coeffs, sizeof(config->ae_2500_config.ae.ae_ccm.mat_coeffs.coeffs));

	params->use.acc_ae = 1;
}

static void ispAwbEncode(aic_config *config, ipu3_uapi_params *params)
{
	params->acc_param.awb.config.grid.block_height_log2 = (__u16)config->awb_2500_config.awb.grid.grid_block_height;
	params->acc_param.awb.config.grid.block_width_log2 = (__u16)config->awb_2500_config.awb.grid.grid_block_width;
	params->acc_param.awb.config.grid.height = config->awb_2500_config.awb.grid.grid_height;
	params->acc_param.awb.config.grid.width = config->awb_2500_config.awb.grid.grid_width;
	params->acc_param.awb.config.grid.x_start = config->awb_2500_config.awb.grid.grid_x_start;
	params->acc_param.awb.config.grid.y_start = config->awb_2500_config.awb.grid.grid_y_start;

	params->acc_param.awb.config.rgbs_thr_b = config->awb_2500_config.awb.rgbs_B_threshold | IPU3_UAPI_AWB_RGBS_THR_B_EN | IPU3_UAPI_AWB_RGBS_THR_B_INCL_SAT;
	params->acc_param.awb.config.rgbs_thr_gb = config->awb_2500_config.awb.rgbs_Gb_threshold;
	params->acc_param.awb.config.rgbs_thr_gr = config->awb_2500_config.awb.rgbs_Gr_threshold;
	params->acc_param.awb.config.rgbs_thr_r = config->awb_2500_config.awb.rgbs_R_threshold;

	params->use.acc_awb = 1;
}

void ParameterEncoder::encode(aic_config *config, ipu3_uapi_params *params)
{
	/*
	 * Do not encode parameters until the algorithms are run,
	 * or assertions will be hit
	 */
	return;

	ispAwbFrEncode(config, params);
	ispAeEncode(config, params);
	ispAwbEncode(config, params);

	return;
}

} /* namespace libcamera */
