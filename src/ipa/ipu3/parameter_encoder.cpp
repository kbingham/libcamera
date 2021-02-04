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

/* Auto Focus */
#define AF_MAX_GRID_CELLS_IN_ONE_SET 32

/* Linearization Vmem */
#define SH_CSS_BAYER_BITS 11
#define LIN_MAX_VALUE (1 << SH_CSS_BAYER_BITS)

/* Imported directly from CommonUtilMacros.h */
#ifndef MEMCPY_S
#define MEMCPY_S(dest, dmax, src, smax) memcpy((dest), (src), std::min((size_t)(dmax), (size_t)(smax)))
#endif
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

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

static void ispAfEncode(aic_config *config, ipu3_uapi_params *params)
{
	params->acc_param.af.filter_config.y1_coeff_0.a1 = config->af_2500_config.af.y1_coeffs.A1;
	params->acc_param.af.filter_config.y1_coeff_0.a2 = config->af_2500_config.af.y1_coeffs.A2;
	params->acc_param.af.filter_config.y1_coeff_0.a3 = config->af_2500_config.af.y1_coeffs.A3;
	params->acc_param.af.filter_config.y1_coeff_0.a4 = config->af_2500_config.af.y1_coeffs.A4;
	params->acc_param.af.filter_config.y1_coeff_1.a5 = config->af_2500_config.af.y1_coeffs.A5;
	params->acc_param.af.filter_config.y1_coeff_1.a6 = config->af_2500_config.af.y1_coeffs.A6;
	params->acc_param.af.filter_config.y1_coeff_1.a7 = config->af_2500_config.af.y1_coeffs.A7;
	params->acc_param.af.filter_config.y1_coeff_1.a8 = config->af_2500_config.af.y1_coeffs.A8;
	params->acc_param.af.filter_config.y1_coeff_2.a9 = config->af_2500_config.af.y1_coeffs.A9;
	params->acc_param.af.filter_config.y1_coeff_2.a10 = config->af_2500_config.af.y1_coeffs.A10;
	params->acc_param.af.filter_config.y1_coeff_2.a11 = config->af_2500_config.af.y1_coeffs.A11;
	params->acc_param.af.filter_config.y1_coeff_2.a12 = config->af_2500_config.af.y1_coeffs.A12;
	params->acc_param.af.filter_config.y1_sign_vec = config->af_2500_config.af.y1_coeffs.sign_vec;

	params->acc_param.af.filter_config.y2_coeff_0.a1 = config->af_2500_config.af.y2_coeffs.A1;
	params->acc_param.af.filter_config.y2_coeff_0.a2 = config->af_2500_config.af.y2_coeffs.A2;
	params->acc_param.af.filter_config.y2_coeff_0.a3 = config->af_2500_config.af.y2_coeffs.A3;
	params->acc_param.af.filter_config.y2_coeff_0.a4 = config->af_2500_config.af.y2_coeffs.A4;
	params->acc_param.af.filter_config.y2_coeff_1.a5 = config->af_2500_config.af.y2_coeffs.A5;
	params->acc_param.af.filter_config.y2_coeff_1.a6 = config->af_2500_config.af.y2_coeffs.A6;
	params->acc_param.af.filter_config.y2_coeff_1.a7 = config->af_2500_config.af.y2_coeffs.A7;
	params->acc_param.af.filter_config.y2_coeff_1.a8 = config->af_2500_config.af.y2_coeffs.A8;
	params->acc_param.af.filter_config.y2_coeff_2.a9 = config->af_2500_config.af.y2_coeffs.A9;
	params->acc_param.af.filter_config.y2_coeff_2.a10 = config->af_2500_config.af.y2_coeffs.A10;
	params->acc_param.af.filter_config.y2_coeff_2.a11 = config->af_2500_config.af.y2_coeffs.A11;
	params->acc_param.af.filter_config.y2_coeff_2.a12 = config->af_2500_config.af.y2_coeffs.A12;
	params->acc_param.af.filter_config.y2_sign_vec = config->af_2500_config.af.y2_coeffs.sign_vec;

	params->acc_param.af.filter_config.y_calc.y_gen_rate_gr = config->af_2500_config.af.y_calc.y_gen_rate_gr;
	params->acc_param.af.filter_config.y_calc.y_gen_rate_r = config->af_2500_config.af.y_calc.y_gen_rate_r;
	params->acc_param.af.filter_config.y_calc.y_gen_rate_b = config->af_2500_config.af.y_calc.y_gen_rate_b;
	params->acc_param.af.filter_config.y_calc.y_gen_rate_gb = config->af_2500_config.af.y_calc.y_gen_rate_gb;

	params->acc_param.af.filter_config.nf.y1_nf = config->af_2500_config.af.nf.y1_nf;
	params->acc_param.af.filter_config.nf.y2_nf = config->af_2500_config.af.nf.y2_nf;

	params->acc_param.af.grid_cfg.width = config->af_2500_config.af.grid.grid_width;
	params->acc_param.af.grid_cfg.height = config->af_2500_config.af.grid.grid_height;
	params->acc_param.af.grid_cfg.block_width_log2 = config->af_2500_config.af.grid.block_width;
	params->acc_param.af.grid_cfg.block_height_log2 = config->af_2500_config.af.grid.block_height;

	assert(config->af_2500_config.af.grid.grid_width);
	params->acc_param.af.grid_cfg.height_per_slice =
		(unsigned char)(AF_MAX_GRID_CELLS_IN_ONE_SET / config->af_2500_config.af.grid.grid_width);
	params->acc_param.af.grid_cfg.x_start = config->af_2500_config.af.grid.x_start;
	params->acc_param.af.grid_cfg.y_start = config->af_2500_config.af.grid.y_start | IPU3_UAPI_GRID_Y_START_EN;

	params->use.acc_af = 1;
}

static void ispLinVmemEncode(aic_config *config, ipu3_uapi_params *params)
{
	for (unsigned int i = 0; i < LIN_SEGMENTS; i++) {
		params->lin_vmem_params.lin_lutlow_gr[i] = MIN(LIN_MAX_VALUE - 1, config->lin_2500_config.curve_lut_GR[i]);
		params->lin_vmem_params.lin_lutlow_r[i] = MIN(LIN_MAX_VALUE - 1, config->lin_2500_config.curve_lut_R[i]);
		params->lin_vmem_params.lin_lutlow_b[i] = MIN(LIN_MAX_VALUE - 1, config->lin_2500_config.curve_lut_B[i]);
		params->lin_vmem_params.lin_lutlow_gb[i] = MIN(LIN_MAX_VALUE - 1, config->lin_2500_config.curve_lut_GB[i]);

		params->lin_vmem_params.lin_lutdif_gr[i] = config->lin_2500_config.curve_lut_GR[i + 1] - config->lin_2500_config.curve_lut_GR[i];
		params->lin_vmem_params.lin_lutdif_r[i] = config->lin_2500_config.curve_lut_R[i + 1] - config->lin_2500_config.curve_lut_R[i];
		params->lin_vmem_params.lin_lutdif_b[i] = config->lin_2500_config.curve_lut_B[i + 1] - config->lin_2500_config.curve_lut_B[i];
		params->lin_vmem_params.lin_lutdif_gb[i] = config->lin_2500_config.curve_lut_GB[i + 1] - config->lin_2500_config.curve_lut_GB[i];
	}

	params->use.lin_vmem_params = 1;
}

static void ispGammaCtrlEncode(aic_config *config, ipu3_uapi_params *params)
{
	params->acc_param.gamma.gc_ctrl.enable = config->rgbpp_2500_config.gamma.enable;
	MEMCPY_S(params->acc_param.gamma.gc_lut.lut, sizeof(params->acc_param.gamma.gc_lut.lut),
		 config->rgbpp_2500_config.gamma.lut_entries, sizeof(config->rgbpp_2500_config.gamma.lut_entries));

	params->use.acc_gamma = 1;
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
	ispAfEncode(config, params);
	ispLinVmemEncode(config, params);
	ispGammaCtrlEncode(config, params);

	return;
}

} /* namespace libcamera */
