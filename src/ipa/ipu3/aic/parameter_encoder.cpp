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

/* Bayer Shading Correction */
#define SHD_MAX_CELLS_PER_SET 146
#define SHD_MAX_CFG_SETS IPU3_UAPI_SHD_MAX_CFG_SETS

/* Iefd */
#define XY_2_RESET_MASK ((1 << 24) - 1)

/* Chroma Noise Reduction */
#define DALPHA_MAX 256

/* Advanced Noise reduction */
#define SQRT_LUT                              \
	{                                     \
		724, 768, 810, 849, 887,      \
		923, 958, 991, 1024, 1056,    \
		1086, 1116, 1145, 1173, 1201, \
		1228, 1254, 1280, 1305, 1330, \
		1355, 1379, 1402, 1425, 1448  \
	}
#define X_SQR_RESET_MAX (0xffffff)
#define Y_SQR_RESET_MAX X_SQR_RESET_MAX
#define R_NORM_FACTOR_MAX (0x1f)
#define RAD_GAIN_SCALE_FACTOR (0xff)
#define COLOR_REG_W_MASK 0xfff
#define COLOR_BETA_MASK 0x7ff
#define COLOR_ALPHA_MASK 0x1ff

/* Extreme Noise Reduction version 3 */
#define XNR_FILTER_SIZE 9
#define ISP_VEC_ELEMBITS 12
#define IA_CSS_XNR3_SIGMA_SCALE (1 << 10)
#define XNR_ALPHA_SCALE_LOG2 5
#define XNR_CORING_SCALE_LOG2 (ISP_VEC_ELEMBITS - 1)
#define XNR_BLENDING_SCALE_LOG2 (ISP_VEC_ELEMBITS - 1)
#define XNR_MIN_SIGMA (IA_CSS_XNR3_SIGMA_SCALE / 100)
#define XNR_MAX_ALPHA ((1 << (ISP_VEC_ELEMBITS - 1)) - 1)
#define XNR_ALPHA_SCALE_FACTOR (1 << XNR_ALPHA_SCALE_LOG2)
#define XNR_CORING_SCALE_FACTOR (1 << XNR_CORING_SCALE_LOG2)
#define XNR_BLENDING_SCALE_FACTOR (1 << XNR_BLENDING_SCALE_LOG2)

/* Extreme Noise Reduction version 3 Vmem */
#define ISP_VEC_NELEMS 64
#define XNR3_LOOK_UP_TABLE_POINTS 16

static const int16_t x[XNR3_LOOK_UP_TABLE_POINTS] = {
	1024, 1164, 1320, 1492, 1680, 1884, 2108, 2352,
	2616, 2900, 3208, 3540, 3896, 4276, 4684, 5120
};

static const int16_t a[XNR3_LOOK_UP_TABLE_POINTS] = {
	-7213, -5580, -4371, -3421, -2722, -2159, -6950, -5585,
	-4529, -3697, -3010, -2485, -2070, -1727, -1428, 0
};

static const int16_t b[XNR3_LOOK_UP_TABLE_POINTS] = {
	4096, 3603, 3178, 2811, 2497, 2226, 1990, 1783,
	1603, 1446, 1307, 1185, 1077, 981, 895, 819
};

static const int16_t c[XNR3_LOOK_UP_TABLE_POINTS] = {
	1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* Temporal Noise Reduction v3 */
#define TNR3_NUM_POINTS (TNR3_NUM_SEGMENTS + 1)
#define TNR3_KNEE_POINTS (TNR3_NUM_SEGMENTS - 1)
#define TNR3_ISP_SCALE (1 << (ISP_VEC_ELEMBITS - 1))
#define TNR3_RND_OFFSET (TNR3_ISP_SCALE >> 1)
#define TNR3_MAX_VALUE (TNR3_ISP_SCALE - 1)
#define TNR3_MIN_VALUE -(TNR3_ISP_SCALE)
#define HOST_SCALING 0

/* Imported directly from CommonUtilMacros.h */
#ifndef MEMCPY_S
#define MEMCPY_S(dest, dmax, src, smax) memcpy((dest), (src), std::min((size_t)(dmax), (size_t)(smax)))
#endif
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define clamp(a, min_val, max_val) MIN(MAX((a), (min_val)), (max_val))

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

static void ispCcmEncode(aic_config *config, ipu3_uapi_params *params)
{
	params->acc_param.ccm.coeff_m11 = config->rgbpp_2500_config.ccm.matrix_coeffs.m11;
	params->acc_param.ccm.coeff_m12 = config->rgbpp_2500_config.ccm.matrix_coeffs.m12;
	params->acc_param.ccm.coeff_m13 = config->rgbpp_2500_config.ccm.matrix_coeffs.m13;
	params->acc_param.ccm.coeff_o_r = config->rgbpp_2500_config.ccm.offsets.R;
	params->acc_param.ccm.coeff_m21 = config->rgbpp_2500_config.ccm.matrix_coeffs.m21;
	params->acc_param.ccm.coeff_m22 = config->rgbpp_2500_config.ccm.matrix_coeffs.m22;
	params->acc_param.ccm.coeff_m23 = config->rgbpp_2500_config.ccm.matrix_coeffs.m23;
	params->acc_param.ccm.coeff_o_g = config->rgbpp_2500_config.ccm.offsets.G;
	params->acc_param.ccm.coeff_m31 = config->rgbpp_2500_config.ccm.matrix_coeffs.m31;
	params->acc_param.ccm.coeff_m32 = config->rgbpp_2500_config.ccm.matrix_coeffs.m32;
	params->acc_param.ccm.coeff_m33 = config->rgbpp_2500_config.ccm.matrix_coeffs.m33;
	params->acc_param.ccm.coeff_o_b = config->rgbpp_2500_config.ccm.offsets.B;

	params->use.acc_ccm = 1;
}

static void ispCscEncode(aic_config *config, ipu3_uapi_params *params)
{
	params->acc_param.csc.coeff_c11 = config->rgbpp_2500_config.csc.mat.c11;
	params->acc_param.csc.coeff_c12 = config->rgbpp_2500_config.csc.mat.c12;
	params->acc_param.csc.coeff_c13 = config->rgbpp_2500_config.csc.mat.c13;
	params->acc_param.csc.coeff_b1 = config->rgbpp_2500_config.csc.offset.b1;
	params->acc_param.csc.coeff_c21 = config->rgbpp_2500_config.csc.mat.c21;
	params->acc_param.csc.coeff_c22 = config->rgbpp_2500_config.csc.mat.c22;
	params->acc_param.csc.coeff_c23 = config->rgbpp_2500_config.csc.mat.c23;
	params->acc_param.csc.coeff_b2 = config->rgbpp_2500_config.csc.offset.b2;
	params->acc_param.csc.coeff_c31 = config->rgbpp_2500_config.csc.mat.c31;
	params->acc_param.csc.coeff_c32 = config->rgbpp_2500_config.csc.mat.c32;
	params->acc_param.csc.coeff_c33 = config->rgbpp_2500_config.csc.mat.c33;
	params->acc_param.csc.coeff_b3 = config->rgbpp_2500_config.csc.offset.b3;

	params->use.acc_csc = 1;
}

static void ispCdsEncode(aic_config *config, ipu3_uapi_params *params)
{
	params->acc_param.cds.ds_c00 = (__u32)config->rgbpp_2500_config.cds.coeffs.c00;
	params->acc_param.cds.ds_c01 = (__u32)config->rgbpp_2500_config.cds.coeffs.c01;
	params->acc_param.cds.ds_c02 = (__u32)config->rgbpp_2500_config.cds.coeffs.c02;
	params->acc_param.cds.ds_c03 = (__u32)config->rgbpp_2500_config.cds.coeffs.c03;
	params->acc_param.cds.ds_c10 = (__u32)config->rgbpp_2500_config.cds.coeffs.c10;
	params->acc_param.cds.ds_c11 = (__u32)config->rgbpp_2500_config.cds.coeffs.c11;
	params->acc_param.cds.ds_c12 = (__u32)config->rgbpp_2500_config.cds.coeffs.c12;
	params->acc_param.cds.ds_c13 = (__u32)config->rgbpp_2500_config.cds.coeffs.c13;
	params->acc_param.cds.ds_nf = (__u32)config->rgbpp_2500_config.cds.nf;

	params->acc_param.cds.uv_bin_output = 0;
	params->acc_param.cds.csc_en = 1;

	params->use.acc_cds = 1;
}

static void ispDmEncode(aic_config *config, ipu3_uapi_params *params)
{
	params->acc_param.dm.dm_en = 1;
	params->acc_param.dm.ch_ar_en = config->dm_2500_config.dm.ch_ar_en;
	params->acc_param.dm.fcc_en = config->dm_2500_config.dm.fcc_en;
	params->acc_param.dm.gamma_sc = config->dm_2500_config.dm.gamma_sc;
	params->acc_param.dm.lc_ctrl = config->dm_2500_config.dm.lc_ctrl;
	params->acc_param.dm.cr_param1 = config->dm_2500_config.dm.cr_param1;
	params->acc_param.dm.cr_param2 = config->dm_2500_config.dm.cr_param2;
	params->acc_param.dm.coring_param = config->dm_2500_config.dm.coring_param;

	params->use.acc_dm = 1;
}

static void ispShdEncode(aic_config *config, ipu3_uapi_params *params)
{
	params->acc_param.shd.shd.grid.width = config->shd_2500_config.shd.grid.grid_width;
	params->acc_param.shd.shd.grid.height = config->shd_2500_config.shd.grid.grid_height;
	params->acc_param.shd.shd.grid.block_width_log2 = config->shd_2500_config.shd.grid.block_width;
	params->acc_param.shd.shd.grid.block_height_log2 = config->shd_2500_config.shd.grid.block_height;

	assert(config->shd_2500_config.shd.grid.grid_width);
	params->acc_param.shd.shd.grid.grid_height_per_slice =
		(unsigned char)(SHD_MAX_CELLS_PER_SET / config->shd_2500_config.shd.grid.grid_width);
	params->acc_param.shd.shd.grid.x_start = config->shd_2500_config.shd.grid.x_start;
	params->acc_param.shd.shd.grid.y_start = config->shd_2500_config.shd.grid.y_start;

	params->acc_param.shd.shd.general.shd_enable = config->shd_2500_config.shd.general.shd_enable;
	params->acc_param.shd.shd.general.gain_factor = config->shd_2500_config.shd.general.gain_factor;
	params->acc_param.shd.shd.general.init_set_vrt_offst_ul =
		(-(signed int)config->shd_2500_config.shd.grid.y_start >> (signed int)config->shd_2500_config.shd.grid.block_height) % (signed int)params->acc_param.shd.shd.grid.grid_height_per_slice;

	params->acc_param.shd.shd.black_level.bl_r = config->shd_2500_config.shd.black_level.bl_R;
	params->acc_param.shd.shd.black_level.bl_gr = config->shd_2500_config.shd.black_level.bl_Gr | (config->shd_2500_config.shd.black_level.normalization_shift << IPU3_UAPI_SHD_BLGR_NF_SHIFT);
	params->acc_param.shd.shd.black_level.bl_gb = config->shd_2500_config.shd.black_level.bl_Gb;
	params->acc_param.shd.shd.black_level.bl_b = config->shd_2500_config.shd.black_level.bl_B;

	unsigned int set_grid_limit =
		config->shd_2500_config.shd.grid.grid_width * params->acc_param.shd.shd.grid.grid_height_per_slice;
	unsigned int public_grid_limit =
		config->shd_2500_config.shd.grid.grid_width * config->shd_2500_config.shd.grid.grid_height;
	unsigned int set_index, set_grid_index, public_grid_index = 0;

	for (set_index = 0; set_index < SHD_MAX_CFG_SETS; set_index++) {
		for (set_grid_index = 0;
		     set_grid_index < set_grid_limit && public_grid_index < public_grid_limit;
		     set_grid_index++, public_grid_index++) {
			params->acc_param.shd.shd_lut.sets[set_index].r_and_gr[set_grid_index].r =
				config->shd_2500_config.shd.luts.R[public_grid_index];
			params->acc_param.shd.shd_lut.sets[set_index].r_and_gr[set_grid_index].gr =
				config->shd_2500_config.shd.luts.Gr[public_grid_index];
			params->acc_param.shd.shd_lut.sets[set_index].gb_and_b[set_grid_index].gb =
				config->shd_2500_config.shd.luts.Gb[public_grid_index];
			params->acc_param.shd.shd_lut.sets[set_index].gb_and_b[set_grid_index].b =
				config->shd_2500_config.shd.luts.B[public_grid_index];
		}
	}

	params->use.acc_shd = 1;
}

static void ispIefdEncode(aic_config *config, ipu3_uapi_params *params)
{
	CLEAR(params->acc_param.iefd);

	params->acc_param.iefd.control.iefd_en = config->yuvp1_c0_2500_config.iefd.control.iefd_en ? 1 : 0;
	params->acc_param.iefd.control.rad_en = config->yuvp1_c0_2500_config.iefd.control.iefd_radial_en ? 1 : 0;
	params->acc_param.iefd.control.denoise_en = config->yuvp1_c0_2500_config.iefd.control.iefd_denoise_en ? 1 : 0;
	params->acc_param.iefd.control.direct_smooth_en = config->yuvp1_c0_2500_config.iefd.control.iefd_dir_en ? 1 : 0;

	params->acc_param.iefd.control.vssnlm_en = 1;

	params->acc_param.iefd.units.cu_1.x0 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_1.x[0];
	params->acc_param.iefd.units.cu_1.x1 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_1.x[1];
	params->acc_param.iefd.units.cu_1.a01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_1.a[0];
	params->acc_param.iefd.units.cu_1.b01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_1.b[0];

	params->acc_param.iefd.units.cu_ed.x0 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.x[0];
	params->acc_param.iefd.units.cu_ed.x1 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.x[1];
	params->acc_param.iefd.units.cu_ed.x2 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.x[2];
	params->acc_param.iefd.units.cu_ed.x3 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.x[3];
	params->acc_param.iefd.units.cu_ed.x4 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.x[4];
	params->acc_param.iefd.units.cu_ed.x5 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.x[5];

	params->acc_param.iefd.units.cu_ed.a01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.a[0];
	params->acc_param.iefd.units.cu_ed.a12 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.a[1];
	params->acc_param.iefd.units.cu_ed.a23 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.a[2];
	params->acc_param.iefd.units.cu_ed.a34 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.a[3];
	params->acc_param.iefd.units.cu_ed.a45 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.a[4];

	params->acc_param.iefd.units.cu_ed.b01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.b[0];
	params->acc_param.iefd.units.cu_ed.b12 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.b[1];
	params->acc_param.iefd.units.cu_ed.b23 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.b[2];
	params->acc_param.iefd.units.cu_ed.b34 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.b[3];
	params->acc_param.iefd.units.cu_ed.b45 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_ed.b[4];

	params->acc_param.iefd.units.cu_3.x0 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_3.x[0];
	params->acc_param.iefd.units.cu_3.x1 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_3.x[1];
	params->acc_param.iefd.units.cu_3.a01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_3.a[0];
	params->acc_param.iefd.units.cu_3.b01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_3.b[0];

	params->acc_param.iefd.units.cu_5.x0 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_5.x[0];
	params->acc_param.iefd.units.cu_5.x1 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_5.x[1];
	params->acc_param.iefd.units.cu_5.a01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_5.a[0];
	params->acc_param.iefd.units.cu_5.b01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_5.b[0];

	params->acc_param.iefd.units.cu_6.x0 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_6.x[0];
	params->acc_param.iefd.units.cu_6.x1 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_6.x[1];
	params->acc_param.iefd.units.cu_6.x2 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_6.x[2];
	params->acc_param.iefd.units.cu_6.x3 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_6.x[3];
	params->acc_param.iefd.units.cu_6.a01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_6.a[0];
	params->acc_param.iefd.units.cu_6.a12 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_6.a[1];
	params->acc_param.iefd.units.cu_6.a23 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_6.a[2];
	params->acc_param.iefd.units.cu_6.b01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_6.b[0];
	params->acc_param.iefd.units.cu_6.b12 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_6.b[1];
	params->acc_param.iefd.units.cu_6.b23 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_6.b[2];

	params->acc_param.iefd.units.cu_7.x0 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_7.x[0];
	params->acc_param.iefd.units.cu_7.x1 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_7.x[1];
	params->acc_param.iefd.units.cu_7.a01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_7.a[0];
	params->acc_param.iefd.units.cu_7.b01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_7.b[0];

	params->acc_param.iefd.units.cu_unsharp.x0 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_unsharp.x[0];
	params->acc_param.iefd.units.cu_unsharp.x1 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_unsharp.x[1];
	params->acc_param.iefd.units.cu_unsharp.x2 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_unsharp.x[2];
	params->acc_param.iefd.units.cu_unsharp.x3 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_unsharp.x[3];
	params->acc_param.iefd.units.cu_unsharp.a01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_unsharp.a[0];
	params->acc_param.iefd.units.cu_unsharp.a12 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_unsharp.a[1];
	params->acc_param.iefd.units.cu_unsharp.a23 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_unsharp.a[2];
	params->acc_param.iefd.units.cu_unsharp.b01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_unsharp.b[0];
	params->acc_param.iefd.units.cu_unsharp.b12 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_unsharp.b[1];
	params->acc_param.iefd.units.cu_unsharp.b23 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_unsharp.b[2];

	params->acc_param.iefd.units.cu_radial.x0 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.x[0];
	params->acc_param.iefd.units.cu_radial.x1 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.x[1];
	params->acc_param.iefd.units.cu_radial.x2 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.x[2];
	params->acc_param.iefd.units.cu_radial.x3 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.x[3];
	params->acc_param.iefd.units.cu_radial.x4 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.x[4];
	params->acc_param.iefd.units.cu_radial.x5 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.x[5];
	params->acc_param.iefd.units.cu_radial.a01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.a[0];
	params->acc_param.iefd.units.cu_radial.a12 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.a[1];
	params->acc_param.iefd.units.cu_radial.a23 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.a[2];
	params->acc_param.iefd.units.cu_radial.a34 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.a[3];
	params->acc_param.iefd.units.cu_radial.a45 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.a[4];
	params->acc_param.iefd.units.cu_radial.b01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.b[0];
	params->acc_param.iefd.units.cu_radial.b12 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.b[1];
	params->acc_param.iefd.units.cu_radial.b23 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.b[2];
	params->acc_param.iefd.units.cu_radial.b34 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.b[3];
	params->acc_param.iefd.units.cu_radial.b45 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_radial.b[4];

	params->acc_param.iefd.units.cu_vssnlm.x0 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_vssnlm.x[0];
	params->acc_param.iefd.units.cu_vssnlm.x1 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_vssnlm.x[1];
	params->acc_param.iefd.units.cu_vssnlm.a01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_vssnlm.a[0];
	params->acc_param.iefd.units.cu_vssnlm.b01 = config->yuvp1_c0_2500_config.iefd.cfg_units.cu_vssnlm.b[0];

	params->acc_param.iefd.config.clamp_stitch = config->yuvp1_c0_2500_config.iefd.config.clamp_stitch;
	params->acc_param.iefd.config.direct_metric_update = config->yuvp1_c0_2500_config.iefd.config.direct_metric_update;
	params->acc_param.iefd.config.horver_diag_coeff = config->yuvp1_c0_2500_config.iefd.config.horver_diag_coeff;
	params->acc_param.iefd.config.ed_horver_diag_coeff = config->yuvp1_c0_2500_config.iefd.config.ed_horver_diag_coeff;

	params->acc_param.iefd.sharp.cfg.nega_lmt_txt = config->yuvp1_c0_2500_config.iefd.sharp.cfg.nega_lmt_txt;
	params->acc_param.iefd.sharp.cfg.posi_lmt_txt = config->yuvp1_c0_2500_config.iefd.sharp.cfg.posi_lmt_txt;
	params->acc_param.iefd.sharp.cfg.nega_lmt_dir = config->yuvp1_c0_2500_config.iefd.sharp.cfg.nega_lmt_dir;
	params->acc_param.iefd.sharp.cfg.posi_lmt_dir = config->yuvp1_c0_2500_config.iefd.sharp.cfg.posi_lmt_dir;

	params->acc_param.iefd.sharp.far_w.dir_shrp = config->yuvp1_c0_2500_config.iefd.sharp.far_w.dir_shrp;
	params->acc_param.iefd.sharp.far_w.dir_dns = config->yuvp1_c0_2500_config.iefd.sharp.far_w.dir_dns;
	params->acc_param.iefd.sharp.far_w.ndir_dns_powr = config->yuvp1_c0_2500_config.iefd.sharp.far_w.ndir_dns_powr;
	params->acc_param.iefd.sharp.unshrp_cfg.unsharp_weight = config->yuvp1_c0_2500_config.iefd.sharp.unshrp_cfg.unsharp_weight;
	params->acc_param.iefd.sharp.unshrp_cfg.unsharp_amount = config->yuvp1_c0_2500_config.iefd.sharp.unshrp_cfg.unsharp_amount;

	params->acc_param.iefd.unsharp.unsharp_coef0.c00 = config->yuvp1_c0_2500_config.iefd.unsharp.unsharp_coef0.c00;
	params->acc_param.iefd.unsharp.unsharp_coef0.c01 = config->yuvp1_c0_2500_config.iefd.unsharp.unsharp_coef0.c01;
	params->acc_param.iefd.unsharp.unsharp_coef0.c02 = config->yuvp1_c0_2500_config.iefd.unsharp.unsharp_coef0.c02;
	params->acc_param.iefd.unsharp.unsharp_coef1.c11 = config->yuvp1_c0_2500_config.iefd.unsharp.unsharp_coef1.c11;
	params->acc_param.iefd.unsharp.unsharp_coef1.c12 = config->yuvp1_c0_2500_config.iefd.unsharp.unsharp_coef1.c12;
	params->acc_param.iefd.unsharp.unsharp_coef1.c22 = config->yuvp1_c0_2500_config.iefd.unsharp.unsharp_coef1.c22;

	params->acc_param.iefd.rad.reset_xy.x = config->yuvp1_c0_2500_config.iefd.rad.reset.x;
	params->acc_param.iefd.rad.reset_xy.y = config->yuvp1_c0_2500_config.iefd.rad.reset.y;
	params->acc_param.iefd.rad.reset_x2.x2 = (config->yuvp1_c0_2500_config.iefd.rad.reset.x *
						  config->yuvp1_c0_2500_config.iefd.rad.reset.x) &
						 XY_2_RESET_MASK;
	params->acc_param.iefd.rad.reset_y2.y2 = (config->yuvp1_c0_2500_config.iefd.rad.reset.y *
						  config->yuvp1_c0_2500_config.iefd.rad.reset.y) &
						 XY_2_RESET_MASK;

	params->acc_param.iefd.rad.cfg.rad_nf = config->yuvp1_c0_2500_config.iefd.rad.cfg.rad_nf;
	params->acc_param.iefd.rad.cfg.rad_inv_r2 = config->yuvp1_c0_2500_config.iefd.rad.cfg.rad_inv_r2;
	params->acc_param.iefd.rad.rad_far_w.rad_dir_far_sharp_w = config->yuvp1_c0_2500_config.iefd.rad.rad_far_w.rad_dir_far_sharp_w;
	params->acc_param.iefd.rad.rad_far_w.rad_dir_far_dns_w = config->yuvp1_c0_2500_config.iefd.rad.rad_far_w.rad_dir_far_dns_w;
	params->acc_param.iefd.rad.rad_far_w.rad_ndir_far_dns_power = config->yuvp1_c0_2500_config.iefd.rad.rad_far_w.rad_ndir_far_dns_power;

	params->acc_param.iefd.rad.cu_cfg0.cu6_pow = config->yuvp1_c0_2500_config.iefd.rad.cu_cfg0.cu6_pow;
	params->acc_param.iefd.rad.cu_cfg0.cu_unsharp_pow = config->yuvp1_c0_2500_config.iefd.rad.cu_cfg0.cu_unsharp_pow;
	params->acc_param.iefd.rad.cu_cfg0.rad_cu6_pow = config->yuvp1_c0_2500_config.iefd.rad.cu_cfg0.rad_cu6_pow;
	params->acc_param.iefd.rad.cu_cfg0.rad_cu_unsharp_pow = config->yuvp1_c0_2500_config.iefd.rad.cu_cfg0.rad_cu_unsharp_pow;

	params->acc_param.iefd.rad.cu_cfg1.rad_cu6_x1 = config->yuvp1_c0_2500_config.iefd.rad.cu_cfg1.rad_cu6_x1;
	params->acc_param.iefd.rad.cu_cfg1.rad_cu_unsharp_x1 = config->yuvp1_c0_2500_config.iefd.rad.cu_cfg1.rad_cu_unsharp_x1;

	if (params->acc_param.iefd.control.vssnlm_en) {
		params->acc_param.iefd.vsslnm.vss_lut_x.vs_x0 = config->yuvp1_c0_2500_config.iefd.vsslnm.vss_lut_x.vs_x0;
		params->acc_param.iefd.vsslnm.vss_lut_x.vs_x1 = config->yuvp1_c0_2500_config.iefd.vsslnm.vss_lut_x.vs_x1;
		params->acc_param.iefd.vsslnm.vss_lut_x.vs_x2 = config->yuvp1_c0_2500_config.iefd.vsslnm.vss_lut_x.vs_x2;

		params->acc_param.iefd.vsslnm.vss_lut_y.vs_y1 = config->yuvp1_c0_2500_config.iefd.vsslnm.vss_lut_y.vs_y1;
		params->acc_param.iefd.vsslnm.vss_lut_y.vs_y2 = config->yuvp1_c0_2500_config.iefd.vsslnm.vss_lut_y.vs_y2;
		params->acc_param.iefd.vsslnm.vss_lut_y.vs_y3 = config->yuvp1_c0_2500_config.iefd.vsslnm.vss_lut_y.vs_y3;
	}

	params->use.acc_iefd = 1;
}

static void ispYdsEncode(aic_config *config, ipu3_uapi_params *params)
{
	params->acc_param.yds.c00 = config->yuvp1_2500_config.yds.c00;
	params->acc_param.yds.c01 = config->yuvp1_2500_config.yds.c01;
	params->acc_param.yds.c02 = config->yuvp1_2500_config.yds.c02;
	params->acc_param.yds.c03 = config->yuvp1_2500_config.yds.c03;
	params->acc_param.yds.c10 = config->yuvp1_2500_config.yds.c10;
	params->acc_param.yds.c11 = config->yuvp1_2500_config.yds.c11;
	params->acc_param.yds.c12 = config->yuvp1_2500_config.yds.c12;
	params->acc_param.yds.c13 = config->yuvp1_2500_config.yds.c13;
	params->acc_param.yds.norm_factor = config->yuvp1_2500_config.yds.norm_factor;
	params->acc_param.yds.bin_output = config->yuvp1_2500_config.yds.bin_output;

	params->use.acc_yds = 1;
}

static void ispYdsC0Encode(aic_config *config, ipu3_uapi_params *params)
{
	params->acc_param.yds_c0.c00 = config->yuvp1_c0_2500_config.yds_c0.c00;
	params->acc_param.yds_c0.c01 = config->yuvp1_c0_2500_config.yds_c0.c01;
	params->acc_param.yds_c0.c02 = config->yuvp1_c0_2500_config.yds_c0.c02;
	params->acc_param.yds_c0.c03 = config->yuvp1_c0_2500_config.yds_c0.c03;
	params->acc_param.yds_c0.c10 = config->yuvp1_c0_2500_config.yds_c0.c10;
	params->acc_param.yds_c0.c11 = config->yuvp1_c0_2500_config.yds_c0.c11;
	params->acc_param.yds_c0.c12 = config->yuvp1_c0_2500_config.yds_c0.c12;
	params->acc_param.yds_c0.c13 = config->yuvp1_c0_2500_config.yds_c0.c13;
	params->acc_param.yds_c0.norm_factor = config->yuvp1_c0_2500_config.yds_c0.norm_factor;
	params->acc_param.yds_c0.bin_output = config->yuvp1_c0_2500_config.yds_c0.bin_output;

	params->use.acc_yds_c0 = 1;
}

static void ispYds2Encode(aic_config *config, ipu3_uapi_params *params)
{
	params->acc_param.yds2.c00 = config->yuvp2_2500_config.yds2.c00;
	params->acc_param.yds2.c01 = config->yuvp2_2500_config.yds2.c01;
	params->acc_param.yds2.c02 = config->yuvp2_2500_config.yds2.c02;
	params->acc_param.yds2.c03 = config->yuvp2_2500_config.yds2.c03;
	params->acc_param.yds2.c10 = config->yuvp2_2500_config.yds2.c10;
	params->acc_param.yds2.c11 = config->yuvp2_2500_config.yds2.c11;
	params->acc_param.yds2.c12 = config->yuvp2_2500_config.yds2.c12;
	params->acc_param.yds2.c13 = config->yuvp2_2500_config.yds2.c13;
	params->acc_param.yds2.norm_factor = config->yuvp2_2500_config.yds2.norm_factor;
	params->acc_param.yds2.bin_output = config->yuvp2_2500_config.yds2.bin_output;

	params->use.acc_yds2 = 1;
}

static void ispChnrEncode(aic_config *config, ipu3_uapi_params *params)
{
	CLEAR(params->acc_param.chnr);

	params->acc_param.chnr.coring.u = config->yuvp1_2500_config.chnr.coring.u;
	params->acc_param.chnr.coring.v = config->yuvp1_2500_config.chnr.coring.v;

	params->acc_param.chnr.sense_gain.vy = config->yuvp1_2500_config.chnr.sense_gain.vy;
	params->acc_param.chnr.sense_gain.vu = config->yuvp1_2500_config.chnr.sense_gain.vu;
	params->acc_param.chnr.sense_gain.vv = config->yuvp1_2500_config.chnr.sense_gain.vv;
	params->acc_param.chnr.sense_gain.hy = config->yuvp1_2500_config.chnr.sense_gain.hy;
	params->acc_param.chnr.sense_gain.hu = config->yuvp1_2500_config.chnr.sense_gain.hu;
	params->acc_param.chnr.sense_gain.hv = config->yuvp1_2500_config.chnr.sense_gain.hv;

	params->acc_param.chnr.iir_fir.fir_0h = config->yuvp1_2500_config.chnr.iir_fir.fir_0h;
	params->acc_param.chnr.iir_fir.fir_1h = config->yuvp1_2500_config.chnr.iir_fir.fir_1h;
	params->acc_param.chnr.iir_fir.fir_2h = config->yuvp1_2500_config.chnr.iir_fir.fir_2h;
	params->acc_param.chnr.iir_fir.dalpha_clip_val = DALPHA_MAX - config->yuvp1_2500_config.chnr.iir_fir.iir_min_prev;

	params->use.acc_chnr = 1;
}

static void
ispChnrC0Encode(aic_config *config, ipu3_uapi_params *params)
{
	CLEAR(params->acc_param.chnr_c0);

	params->acc_param.chnr_c0.coring.u = config->yuvp1_c0_2500_config.chnr_c0.coring.u;
	params->acc_param.chnr_c0.coring.v = config->yuvp1_c0_2500_config.chnr_c0.coring.v;

	params->acc_param.chnr_c0.sense_gain.vy = config->yuvp1_c0_2500_config.chnr_c0.sense_gain.vy;
	params->acc_param.chnr_c0.sense_gain.vu = config->yuvp1_c0_2500_config.chnr_c0.sense_gain.vu;
	params->acc_param.chnr_c0.sense_gain.vv = config->yuvp1_c0_2500_config.chnr_c0.sense_gain.vv;
	params->acc_param.chnr_c0.sense_gain.hy = config->yuvp1_c0_2500_config.chnr_c0.sense_gain.hy;
	params->acc_param.chnr_c0.sense_gain.hu = config->yuvp1_c0_2500_config.chnr_c0.sense_gain.hu;
	params->acc_param.chnr_c0.sense_gain.hv = config->yuvp1_c0_2500_config.chnr_c0.sense_gain.hv;

	params->acc_param.chnr_c0.iir_fir.fir_0h = config->yuvp1_c0_2500_config.chnr_c0.iir_fir.fir_0h;
	params->acc_param.chnr_c0.iir_fir.fir_1h = config->yuvp1_c0_2500_config.chnr_c0.iir_fir.fir_1h;
	params->acc_param.chnr_c0.iir_fir.fir_2h = config->yuvp1_c0_2500_config.chnr_c0.iir_fir.fir_2h;
	params->acc_param.chnr_c0.iir_fir.dalpha_clip_val = DALPHA_MAX - config->yuvp1_c0_2500_config.chnr_c0.iir_fir.iir_min_prev;

	params->use.acc_chnr_c0 = 1;
}

static void ispYEeNrEncode(aic_config *config, ipu3_uapi_params *params)
{
	CLEAR(params->acc_param.y_ee_nr);

	params->acc_param.y_ee_nr.lpf.enable = config->yuvp1_2500_config.y_ee_nr.lpf.y_ee_nr_en;
	params->acc_param.y_ee_nr.lpf.a_diag = config->yuvp1_2500_config.y_ee_nr.lpf.a_diag;
	params->acc_param.y_ee_nr.lpf.a_cent = config->yuvp1_2500_config.y_ee_nr.lpf.a_cent;
	params->acc_param.y_ee_nr.lpf.a_periph = config->yuvp1_2500_config.y_ee_nr.lpf.a_periph;

	params->acc_param.y_ee_nr.sense.edge_sense_0 = config->yuvp1_2500_config.y_ee_nr.sense.edge_sense_0;
	params->acc_param.y_ee_nr.sense.delta_edge_sense = config->yuvp1_2500_config.y_ee_nr.sense.delta_edge_sense;
	params->acc_param.y_ee_nr.sense.corner_sense_0 = config->yuvp1_2500_config.y_ee_nr.sense.corner_sense_0;
	params->acc_param.y_ee_nr.sense.delta_corner_sense = config->yuvp1_2500_config.y_ee_nr.sense.delta_corner_sense;

	params->acc_param.y_ee_nr.gain.gain_pos_0 = config->yuvp1_2500_config.y_ee_nr.gain.gain_pos_0;
	params->acc_param.y_ee_nr.gain.delta_gain_posi = config->yuvp1_2500_config.y_ee_nr.gain.delta_gain_posi;
	params->acc_param.y_ee_nr.gain.gain_neg_0 = config->yuvp1_2500_config.y_ee_nr.gain.gain_neg_0;
	params->acc_param.y_ee_nr.gain.delta_gain_neg = config->yuvp1_2500_config.y_ee_nr.gain.delta_gain_neg;

	params->acc_param.y_ee_nr.clip.clip_pos_0 = config->yuvp1_2500_config.y_ee_nr.clip.clip_pos_0;
	params->acc_param.y_ee_nr.clip.delta_clip_posi = config->yuvp1_2500_config.y_ee_nr.clip.delta_clip_posi;
	params->acc_param.y_ee_nr.clip.clip_neg_0 = config->yuvp1_2500_config.y_ee_nr.clip.clip_neg_0;
	params->acc_param.y_ee_nr.clip.delta_clip_neg = config->yuvp1_2500_config.y_ee_nr.clip.delta_clip_neg;

	params->acc_param.y_ee_nr.frng.gain_exp = config->yuvp1_2500_config.y_ee_nr.frng.gain_exp;
	params->acc_param.y_ee_nr.frng.min_edge = config->yuvp1_2500_config.y_ee_nr.frng.min_edge;
	params->acc_param.y_ee_nr.frng.lin_seg_param = config->yuvp1_2500_config.y_ee_nr.frng.lin_seg_param;
	params->acc_param.y_ee_nr.frng.t1 = config->yuvp1_2500_config.y_ee_nr.frng.t1;
	params->acc_param.y_ee_nr.frng.t2 = config->yuvp1_2500_config.y_ee_nr.frng.t2;

	params->acc_param.y_ee_nr.diag.diag_disc_g = config->yuvp1_2500_config.y_ee_nr.diag.diag_disc_g;
	params->acc_param.y_ee_nr.diag.hvw_hor = config->yuvp1_2500_config.y_ee_nr.diag.hvw_hor;
	params->acc_param.y_ee_nr.diag.dw_hor = config->yuvp1_2500_config.y_ee_nr.diag.dw_hor;
	params->acc_param.y_ee_nr.diag.hvw_diag = config->yuvp1_2500_config.y_ee_nr.diag.hvw_diag;
	params->acc_param.y_ee_nr.diag.dw_diag = config->yuvp1_2500_config.y_ee_nr.diag.dw_diag;

	params->acc_param.y_ee_nr.fc_coring.pos_0 = config->yuvp1_2500_config.y_ee_nr.fc_coring.pos_0;
	params->acc_param.y_ee_nr.fc_coring.pos_delta = config->yuvp1_2500_config.y_ee_nr.fc_coring.pos_delta;
	params->acc_param.y_ee_nr.fc_coring.neg_0 = config->yuvp1_2500_config.y_ee_nr.fc_coring.neg_0;
	params->acc_param.y_ee_nr.fc_coring.neg_delta = config->yuvp1_2500_config.y_ee_nr.fc_coring.neg_delta;

	params->use.acc_y_ee_nr = 1;
}

static void ispTccEncode(aic_config *config, ipu3_uapi_params *params)
{
	params->acc_param.tcc.gen_control.en = 1;
	params->acc_param.tcc.gen_control.blend_shift = config->yuvp2_2500_config.tcc.gen_control.blend_shift;
	params->acc_param.tcc.gen_control.delta = config->yuvp2_2500_config.tcc.gen_control.delta;
	params->acc_param.tcc.gen_control.gamma = config->yuvp2_2500_config.tcc.gen_control.gamma;
	params->acc_param.tcc.gen_control.gain_according_to_y_only = config->yuvp2_2500_config.tcc.gen_control.gain_according_to_y_only;

	MEMCPY_S(params->acc_param.tcc.macc_table.entries,
		 sizeof(params->acc_param.tcc.macc_table.entries),
		 config->yuvp2_2500_config.tcc.macc_table.entries,
		 sizeof(config->yuvp2_2500_config.tcc.macc_table.entries));

	MEMCPY_S(params->acc_param.tcc.inv_y_lut.entries,
		 sizeof(params->acc_param.tcc.inv_y_lut.entries),
		 config->yuvp2_2500_config.tcc.inv_y_lut.entries,
		 sizeof(config->yuvp2_2500_config.tcc.inv_y_lut.entries));

	MEMCPY_S(params->acc_param.tcc.gain_pcwl.entries,
		 sizeof(params->acc_param.tcc.gain_pcwl.entries),
		 config->yuvp2_2500_config.tcc.gain_pcwl.entries,
		 sizeof(config->yuvp2_2500_config.tcc.gain_pcwl.entries));

	MEMCPY_S(params->acc_param.tcc.r_sqr_lut.entries,
		 sizeof(params->acc_param.tcc.r_sqr_lut.entries),
		 config->yuvp2_2500_config.tcc.r_sqr_lut.entries,
		 sizeof(config->yuvp2_2500_config.tcc.r_sqr_lut.entries));

	params->use.acc_tcc = 1;
}

static void copyAlpha(ipu3_uapi_anr_alpha *to, alpha_t *from)
{
	to->gr = from->Alpha_Gr & COLOR_ALPHA_MASK;
	to->r = from->Alpha_R & COLOR_ALPHA_MASK;
	to->b = from->Alpha_B & COLOR_ALPHA_MASK;
	to->gb = from->Alpha_Gb & COLOR_ALPHA_MASK;
	to->dc_gr = from->Alpha_DC_Gr & COLOR_ALPHA_MASK;
	to->dc_r = from->Alpha_DC_R & COLOR_ALPHA_MASK;
	to->dc_b = from->Alpha_DC_B & COLOR_ALPHA_MASK;
	to->dc_gb = from->Alpha_DC_Gb & COLOR_ALPHA_MASK;
}

static void copyBeta(ipu3_uapi_anr_beta *to, beta_t *from)
{
	to->beta_gr = from->Beta_Gr & COLOR_BETA_MASK;
	to->beta_r = from->Beta_R & COLOR_BETA_MASK;
	to->beta_b = from->Beta_B & COLOR_BETA_MASK;
	to->beta_gb = from->Beta_Gb & COLOR_BETA_MASK;
}

static void copyColoreRg(ipu3_uapi_anr_plane_color *to, plain_color_w_matrix_t *from)
{
	for (int i = 0; i < W_MATRIX_SIZE; i++) {
		to->reg_w_gr[i] = from->Gr[i] & COLOR_REG_W_MASK;
		to->reg_w_r[i] = from->R[i] & COLOR_REG_W_MASK;
		to->reg_w_b[i] = from->B[i] & COLOR_REG_W_MASK;
		to->reg_w_gb[i] = from->Gb[i] & COLOR_REG_W_MASK;
	}
}

static void ispAnrEncode(aic_config *config, ipu3_uapi_params *params)
{
	CLEAR(params->acc_param.anr);

	short sqrt_lut[] = SQRT_LUT;

	params->acc_param.anr.transform.enable = 1;
	params->acc_param.anr.transform.adaptive_treshhold_en = config->anr_2500_config.anr.transform.ADAPTIVE_TRESHHOLD_EN & 0x1;
	copyAlpha(&params->acc_param.anr.transform.alpha[0], &config->anr_2500_config.anr.transform.plane_0.alpha);
	copyAlpha(&params->acc_param.anr.transform.alpha[1], &config->anr_2500_config.anr.transform.plane_1.alpha);
	copyAlpha(&params->acc_param.anr.transform.alpha[2], &config->anr_2500_config.anr.transform.plane_2.alpha);

	copyBeta(&params->acc_param.anr.transform.beta[0], &config->anr_2500_config.anr.transform.plane_0.beta);
	copyBeta(&params->acc_param.anr.transform.beta[1], &config->anr_2500_config.anr.transform.plane_1.beta);
	copyBeta(&params->acc_param.anr.transform.beta[2], &config->anr_2500_config.anr.transform.plane_2.beta);

	copyColoreRg(&params->acc_param.anr.transform.color[0], &config->anr_2500_config.anr.transform.plane_0.color_reg_w);
	copyColoreRg(&params->acc_param.anr.transform.color[1], &config->anr_2500_config.anr.transform.plane_1.color_reg_w);
	copyColoreRg(&params->acc_param.anr.transform.color[2], &config->anr_2500_config.anr.transform.plane_2.color_reg_w);

	MEMCPY_S(params->acc_param.anr.transform.sqrt_lut,
		 sizeof(params->acc_param.anr.transform.sqrt_lut),
		 &sqrt_lut,
		 sizeof(sqrt_lut));

	params->acc_param.anr.transform.xreset = config->anr_2500_config.anr.transform.CALC.Xreset;
	params->acc_param.anr.transform.yreset = config->anr_2500_config.anr.transform.CALC.Yreset;

	params->acc_param.anr.transform.x_sqr_reset = config->anr_2500_config.anr.transform.CALC.X_sqr_reset;
	if (config->anr_2500_config.anr.transform.CALC.X_sqr_reset & ~(X_SQR_RESET_MAX)) {
		params->acc_param.anr.transform.x_sqr_reset = X_SQR_RESET_MAX;
	}

	params->acc_param.anr.transform.r_normfactor = config->anr_2500_config.anr.transform.CALC.R_NormFactor;
	if (config->anr_2500_config.anr.transform.CALC.R_NormFactor & ~(R_NORM_FACTOR_MAX)) {
		params->acc_param.anr.transform.r_normfactor = R_NORM_FACTOR_MAX;
	}

	params->acc_param.anr.transform.y_sqr_reset = config->anr_2500_config.anr.transform.CALC.Y_sqr_reset;
	if (config->anr_2500_config.anr.transform.CALC.Y_sqr_reset & ~(Y_SQR_RESET_MAX)) {
		params->acc_param.anr.transform.y_sqr_reset = Y_SQR_RESET_MAX;
	}

	params->acc_param.anr.transform.gain_scale = config->anr_2500_config.anr.transform.CALC.radial_gain_scale_factor;
	if (config->anr_2500_config.anr.transform.CALC.radial_gain_scale_factor & ~(RAD_GAIN_SCALE_FACTOR)) {
		params->acc_param.anr.transform.gain_scale = RAD_GAIN_SCALE_FACTOR;
	}
	params->acc_param.anr.stitch.anr_stitch_en = 1;

	for (int i = 0, j = 0; i < IPU3_UAPI_ANR_PYRAMID_SIZE; ++i) {
		params->acc_param.anr.stitch.pyramid[i].entry0 = config->anr_2500_config.anr.stitch.pyramid_reg[j++];

		if (i < IPU3_UAPI_ANR_PYRAMID_SIZE - 1) {
			params->acc_param.anr.stitch.pyramid[i].entry1 = config->anr_2500_config.anr.stitch.pyramid_reg[j++];
			params->acc_param.anr.stitch.pyramid[i].entry2 = config->anr_2500_config.anr.stitch.pyramid_reg[j++];
		}
	}

	params->use.acc_anr = 1;
}

static void ispBnrEncode(aic_config *config, ipu3_uapi_params *params)
{
	CLEAR(params->acc_param.bnr);

	params->acc_param.bnr.wb_gains.gr = config->bnr_2500_config.bnr.wb_gains.gr;
	params->acc_param.bnr.wb_gains.r = config->bnr_2500_config.bnr.wb_gains.r;
	params->acc_param.bnr.wb_gains.b = config->bnr_2500_config.bnr.wb_gains.b;
	params->acc_param.bnr.wb_gains.gb = config->bnr_2500_config.bnr.wb_gains.gb;

	params->acc_param.bnr.wb_gains_thr.gr = (unsigned char)config->bnr_2500_config.bnr.wb_gains_thr.gr;
	params->acc_param.bnr.wb_gains_thr.r = (unsigned char)config->bnr_2500_config.bnr.wb_gains_thr.r;
	params->acc_param.bnr.wb_gains_thr.b = (unsigned char)config->bnr_2500_config.bnr.wb_gains_thr.b;
	params->acc_param.bnr.wb_gains_thr.gb = (unsigned char)config->bnr_2500_config.bnr.wb_gains_thr.gb;

	params->acc_param.bnr.thr_coeffs.cf = config->bnr_2500_config.bnr.thr_coeffs.cf;
	params->acc_param.bnr.thr_coeffs.cg = config->bnr_2500_config.bnr.thr_coeffs.cg;
	params->acc_param.bnr.thr_coeffs.ci = config->bnr_2500_config.bnr.thr_coeffs.ci;
	params->acc_param.bnr.thr_coeffs.r_nf = config->bnr_2500_config.bnr.thr_coeffs.r_nf;

	params->acc_param.bnr.thr_ctrl_shd.gr = config->bnr_2500_config.bnr.thr_ctrl_shd.gr;
	params->acc_param.bnr.thr_ctrl_shd.r = config->bnr_2500_config.bnr.thr_ctrl_shd.r;
	params->acc_param.bnr.thr_ctrl_shd.b = config->bnr_2500_config.bnr.thr_ctrl_shd.b;
	params->acc_param.bnr.thr_ctrl_shd.gb = config->bnr_2500_config.bnr.thr_ctrl_shd.gb;

	params->acc_param.bnr.opt_center.x_reset = config->bnr_2500_config.bnr.opt_center.x_reset;
	params->acc_param.bnr.opt_center.y_reset = config->bnr_2500_config.bnr.opt_center.y_reset;

	params->acc_param.bnr.opt_center_sqr.x_sqr_reset = config->bnr_2500_config.bnr.opt_center.x_sqr_reset;
	params->acc_param.bnr.opt_center_sqr.y_sqr_reset = config->bnr_2500_config.bnr.opt_center.y_sqr_reset;

	MEMCPY_S(params->acc_param.bnr.lut.values,
		 sizeof(params->acc_param.bnr.lut.values),
		 config->bnr_2500_config.bnr.lut.values,
		 sizeof(config->bnr_2500_config.bnr.lut.values));

	params->acc_param.bnr.bp_ctrl.bp_thr_gain = config->bnr_2500_config.bnr.bp_ctrl.bp_thr_gain;
	params->acc_param.bnr.bp_ctrl.defect_mode = config->bnr_2500_config.bnr.bp_ctrl.defect_mode;
	params->acc_param.bnr.bp_ctrl.bp_gain = config->bnr_2500_config.bnr.bp_ctrl.bp_gain;
	params->acc_param.bnr.bp_ctrl.w0_coeff = config->bnr_2500_config.bnr.bp_ctrl.w0_coeff;
	params->acc_param.bnr.bp_ctrl.w1_coeff = config->bnr_2500_config.bnr.bp_ctrl.w1_coeff;

	params->acc_param.bnr.dn_detect_ctrl.alpha = config->bnr_2500_config.bnr.dn_detect_ctrl.alpha;
	params->acc_param.bnr.dn_detect_ctrl.beta = config->bnr_2500_config.bnr.dn_detect_ctrl.beta;
	params->acc_param.bnr.dn_detect_ctrl.gamma = config->bnr_2500_config.bnr.dn_detect_ctrl.gamma;
	params->acc_param.bnr.dn_detect_ctrl.max_inf = config->bnr_2500_config.bnr.dn_detect_ctrl.max_inf;
	params->acc_param.bnr.dn_detect_ctrl.gd_enable = config->bnr_2500_config.bnr.dn_detect_ctrl.gd_en;
	params->acc_param.bnr.dn_detect_ctrl.bpc_enable = config->bnr_2500_config.bnr.dn_detect_ctrl.bpc_en;
	params->acc_param.bnr.dn_detect_ctrl.bnr_enable = config->bnr_2500_config.bnr.dn_detect_ctrl.bnr_en;
	params->acc_param.bnr.dn_detect_ctrl.ff_enable = 1;

	params->use.acc_bnr = 1;
}

static void ispOBGEncode(aic_config *config, ipu3_uapi_params *params)
{
	params->obgrid_param.gr = config->obgrid_2500_config.table_GR[0];
	params->obgrid_param.r = config->obgrid_2500_config.table_R[0];
	params->obgrid_param.b = config->obgrid_2500_config.table_B[0];
	params->obgrid_param.gb = config->obgrid_2500_config.table_GB[0];

	params->use.obgrid = 1;
	params->use.obgrid_param = 1;
}

static void ispBnrGreenDisparityEncode(aic_config *config, ipu3_uapi_params *params)
{
	CLEAR(params->acc_param.green_disparity);

	params->acc_param.green_disparity.gd_red = config->bnr_2500_config.green_disparity.GD_Red;
	params->acc_param.green_disparity.gd_green = config->bnr_2500_config.green_disparity.GD_Green;
	params->acc_param.green_disparity.gd_blue = config->bnr_2500_config.green_disparity.GD_Blue;
	params->acc_param.green_disparity.gd_black = config->bnr_2500_config.green_disparity.GD_Black;
	params->acc_param.green_disparity.gd_shading = config->bnr_2500_config.green_disparity.GD_Shading;
	params->acc_param.green_disparity.gd_support = config->bnr_2500_config.green_disparity.GD_Support;
	params->acc_param.green_disparity.gd_clip = config->bnr_2500_config.green_disparity.GD_Clip;
	params->acc_param.green_disparity.gd_central_weight = config->bnr_2500_config.green_disparity.GD_Central_Weight;

	params->use.acc_green_disparity = 1;
}

static unsigned int ceil_pow2(unsigned int p)
{
	if (p == 0) {
		return 1;
	} else if ((!((p) & ((p)-1)))) {
		return p;
	} else {
		unsigned int v = p;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		return (v + 1);
	}
}

static int32_t compute_alpha(int sigma)
{
	int32_t alpha;
#if defined(XNR_ATE_ROUNDING_BUG)
	int32_t alpha_unscaled;
#else
	int offset = sigma / 2;
#endif
	if (sigma < XNR_MIN_SIGMA) {
		alpha = XNR_MAX_ALPHA;
	} else {
#if defined(XNR_ATE_ROUNDING_BUG)
		/* The scale factor for alpha must be the same as on the ISP,
		 * For sigma, it must match the public interface. The code
		 * below mimics the rounding and unintended loss of precision
		 * of the ATE reference code. It computes an unscaled alpha,
		 * rounds down, and then scales it to get the required fixed
		 * point representation. It would have been more precise to
		 * round after scaling. */
		alpha_unscaled = IA_CSS_XNR3_SIGMA_SCALE / sigma;
		alpha = alpha_unscaled * XNR_ALPHA_SCALE_FACTOR;
#else
		alpha = ((IA_CSS_XNR3_SIGMA_SCALE * XNR_ALPHA_SCALE_FACTOR) + offset) / sigma;
#endif
		if (alpha > XNR_MAX_ALPHA) {
			alpha = XNR_MAX_ALPHA;
		}
	}

	return alpha;
}

static int32_t compute_coring(int coring)
{
	int32_t isp_coring;
	int32_t isp_scale = XNR_CORING_SCALE_FACTOR;
	int32_t host_scale = IA_CSS_XNR3_CORING_SCALE;
	int32_t offset = host_scale / 2; /* fixed-point 0.5 */

	/* Convert from public host-side scale factor to isp-side scale
	 * factor. Clip to [0, isp_scale-1). */
	isp_coring = ((coring * isp_scale) + offset) / host_scale;

	return MIN(MAX(isp_coring, 0), isp_scale - 1);
}

static int32_t compute_blending(int strength)
{
	int32_t isp_strength;
	int32_t isp_scale = XNR_BLENDING_SCALE_FACTOR;
	int32_t host_scale = IA_CSS_XNR3_BLENDING_SCALE;
	int32_t offset = host_scale / 2; /* fixed-point 0.5 */

	/* Convert from public host-side scale factor to isp-side scale
	 * factor. The blending factor is positive on the host side, but
	 * negative on the ISP side because +1.0 cannot be represented
	 * exactly as s0.11 fixed point, but -1.0 can. */
	isp_strength = -(((strength * isp_scale) + offset) / host_scale);

	return MAX(MIN(isp_strength, 0), -XNR_BLENDING_SCALE_FACTOR);
}

static void ispXnr3Encode(aic_config *config, ipu3_uapi_params *params)
{
	CLEAR(params->xnr3_dmem_params);

	struct ipu3_uapi_isp_xnr3_params *to = &params->xnr3_dmem_params;
	const struct ia_css_xnr3_config *from = &config->xnr_2500_config;

	int kernel_size = XNR_FILTER_SIZE;
	/* The adjust factor is the next power of 2 w.r.t. the kernel size */
	int adjust_factor = ceil_pow2(kernel_size);
	int32_t max_diff = (1 << (ISP_VEC_ELEMBITS - 1)) - 1;
	int32_t min_diff = -(1 << (ISP_VEC_ELEMBITS - 1));

	int32_t alpha_y0 = compute_alpha(from->sigma.y0);
	int32_t alpha_y1 = compute_alpha(from->sigma.y1);
	int32_t alpha_u0 = compute_alpha(from->sigma.u0);
	int32_t alpha_u1 = compute_alpha(from->sigma.u1);
	int32_t alpha_v0 = compute_alpha(from->sigma.v0);
	int32_t alpha_v1 = compute_alpha(from->sigma.v1);
	int32_t alpha_ydiff = (alpha_y1 - alpha_y0) * adjust_factor / kernel_size;
	int32_t alpha_udiff = (alpha_u1 - alpha_u0) * adjust_factor / kernel_size;
	int32_t alpha_vdiff = (alpha_v1 - alpha_v0) * adjust_factor / kernel_size;

	int32_t coring_u0 = compute_coring(from->coring.u0);
	int32_t coring_u1 = compute_coring(from->coring.u1);
	int32_t coring_v0 = compute_coring(from->coring.v0);
	int32_t coring_v1 = compute_coring(from->coring.v1);
	int32_t coring_udiff = (coring_u1 - coring_u0) * adjust_factor / kernel_size;
	int32_t coring_vdiff = (coring_v1 - coring_v0) * adjust_factor / kernel_size;

	int32_t blending = compute_blending(from->blending.strength);

	/* alpha's are represented in qN.5 format */
	to->alpha.y0 = alpha_y0;
	to->alpha.u0 = alpha_u0;
	to->alpha.v0 = alpha_v0;
	to->alpha.ydiff = MIN(MAX(alpha_ydiff, min_diff), max_diff);
	to->alpha.udiff = MIN(MAX(alpha_udiff, min_diff), max_diff);
	to->alpha.vdiff = MIN(MAX(alpha_vdiff, min_diff), max_diff);

	/* coring parameters are expressed in q1.NN format */
	to->coring.u0 = coring_u0;
	to->coring.v0 = coring_v0;
	to->coring.udiff = MIN(MAX(coring_udiff, min_diff), max_diff);
	to->coring.vdiff = MIN(MAX(coring_vdiff, min_diff), max_diff);

	/* blending strength is expressed in q1.NN format */
	to->blending.strength = blending;

	params->use.xnr3_dmem_params = 1;
}

static void ispXnr3VmemEncode([[maybe_unused]] aic_config *config, ipu3_uapi_params *params)
{
	struct ipu3_uapi_isp_xnr3_vmem_params *to = &params->xnr3_vmem_params;
	CLEAR(params->xnr3_vmem_params);

	unsigned i, j, base;
	const unsigned total_blocks = 4;
	const unsigned shuffle_block = 16;

	/* Init */
	for (i = 0; i < ISP_VEC_NELEMS; i++) {
		to->x[i] = 0;
		to->a[i] = 0;
		to->b[i] = 0;
		to->c[i] = 0;
	}

	/* Constraints on "x":
	 * - values should be greater or equal to 0.
	 * - values should be ascending.
	 */
	assert(x[0] >= 0);

	for (j = 1; j < XNR3_LOOK_UP_TABLE_POINTS; j++) {
		assert(x[j] >= 0);
		assert(x[j] > x[j - 1]);
	}

	/* The implementation of the calulating 1/x is based on the availability
	 * of the OP_vec_shuffle16 operation.
	 * A 64 element vector is split up in 4 blocks of 16 element. Each array
	 * is copied to a vector 4 times, (starting at 0, 16, 32 and 48). All
	 * array elements are copied or initialised as described in the KFS. The
	 * remaining elements of a vector are set to 0.
	 */
	/* TODO: guard this code with above assumptions */
	for (i = 0; i < total_blocks; i++) {
		base = shuffle_block * i;

		for (j = 0; j < XNR3_LOOK_UP_TABLE_POINTS; j++) {
			to->x[base + j] = x[j];
			to->a[base + j] = a[j];
			to->b[base + j] = b[j];
			to->c[base + j] = c[j];
		}
	}

	params->use.xnr3_vmem_params = 1;
}

static int qrmul(int number1, int number2)
{
	int result;
	int offset = TNR3_RND_OFFSET;
	int prod = number1 * number2;

	if (prod >= 0)
		result = prod + offset;
	else
		result = prod - offset;

	return (result / TNR3_ISP_SCALE);
}

static void ispTnr3VmemEncode(aic_config *config, ipu3_uapi_params *params)
{
	int i, j = 0;
	int xdiff, scale_factor;
	int knee_point[TNR3_NUM_POINTS];
	int slopeu_y[TNR3_NUM_SEGMENTS];
	int slopeu_u[TNR3_NUM_SEGMENTS];
	int slopeu_v[TNR3_NUM_SEGMENTS];
	int normalised_ydiff[TNR3_NUM_SEGMENTS];
	int normalised_udiff[TNR3_NUM_SEGMENTS];
	int normalised_vdiff[TNR3_NUM_SEGMENTS];
	int yintercept_y[TNR3_NUM_SEGMENTS];
	int yintercept_u[TNR3_NUM_SEGMENTS];
	int yintercept_v[TNR3_NUM_SEGMENTS];

	knee_point[0] = 0;
	knee_point[TNR3_NUM_POINTS - 1] = TNR3_MAX_VALUE;

	for (i = 0; i < TNR3_KNEE_POINTS; i++) {
		knee_point[1 + i] = config->tnr3_2500_config.knee_y[i];
	}

	for (i = 0; i < TNR3_NUM_SEGMENTS; i++) {
		/* Calculating slope for Y, U and V. Slope is (y2 - y1)/(x2 - x1).
		 * This division results in a loss of the normalisation coefficient
		 * which causes unacceptable loss in precision. In order to
		 * overcome that, we multiple the ydiff (y2 - y1) by the
		 * normalisation coefficient once again
		 */
		normalised_ydiff[i] = (config->tnr3_2500_config.sigma_y[i + 1] - config->tnr3_2500_config.sigma_y[i]) * TNR3_ISP_SCALE;
		normalised_udiff[i] = (config->tnr3_2500_config.sigma_u[i + 1] - config->tnr3_2500_config.sigma_u[i]) * TNR3_ISP_SCALE;
		normalised_vdiff[i] = (config->tnr3_2500_config.sigma_v[i + 1] - config->tnr3_2500_config.sigma_v[i]) * TNR3_ISP_SCALE;

		/* Calculation of xdiff (x2 - x1) */
		xdiff = knee_point[i + 1] - knee_point[i];

		if (xdiff == 0) { /* Zero length segment */
			slopeu_y[i] = 0;
			slopeu_u[i] = 0;
			slopeu_v[i] = 0;
		} else {
			/* Slope(normalised) = ydiff(normalised)/xdiff. As the slope
			 * should be normalised to ISP_VEC_ELEMBITS, it should be
			 * clipped at the minimum and maximum allowable values.
			 */
			slopeu_y[i] = clamp((normalised_ydiff[i] / xdiff), TNR3_MIN_VALUE, TNR3_MAX_VALUE);
			slopeu_u[i] = clamp((normalised_udiff[i] / xdiff), TNR3_MIN_VALUE, TNR3_MAX_VALUE);
			slopeu_v[i] = clamp((normalised_vdiff[i] / xdiff), TNR3_MIN_VALUE, TNR3_MAX_VALUE);
		}
		/* Calculate Y axis (standard deviation) intercept using the formula
		 * Y1 - m*X1 for each linear segment per plane. To mimic the method
		 * followed in ATE, this calculation is done after clipping the
		 * slope value post normalisation. As the input points are
		 * already normalised, there is no need for clipping the
		 * Y-intercepts.
		 * TODO: ATE does nearest even rounding whereas we do nearest
		 * rounding. We need to modify the ATE code to work with integer
		 * values so that similar rounding mechanisms can be implemented
		 * on both sides
		 */
		yintercept_y[i] = config->tnr3_2500_config.sigma_y[i] - qrmul(slopeu_y[i], knee_point[i]);
		yintercept_u[i] = config->tnr3_2500_config.sigma_u[i] - qrmul(slopeu_u[i], knee_point[i]);
		yintercept_v[i] = config->tnr3_2500_config.sigma_v[i] - qrmul(slopeu_v[i], knee_point[i]);
	}

#if HOST_SCALING
	scale_factor = 2;
#else
	scale_factor = 1;
#endif
	for (i = 0; i < TNR3_NUM_SEGMENTS; i++) {
		j = (TNR3_NUM_SEGMENTS - 1) - i;
		/* Slope */
		/* TODO: Should the scaling be done on Host or ISP ?? */
		params->tnr3_vmem_params.slope[j] = clamp(slopeu_y[i] * scale_factor, TNR3_MIN_VALUE, TNR3_MAX_VALUE);
		params->tnr3_vmem_params.slope[j + TNR3_NUM_SEGMENTS] = clamp(slopeu_u[i] * scale_factor, TNR3_MIN_VALUE, TNR3_MAX_VALUE);
		params->tnr3_vmem_params.slope[j + 2 * TNR3_NUM_SEGMENTS] = clamp(slopeu_v[i] * scale_factor, TNR3_MIN_VALUE, TNR3_MAX_VALUE);
		/* Y intercept */
		/* TODO: Should the scaling be done on HOST or ISP ?? */
		params->tnr3_vmem_params.sigma[j] = clamp(yintercept_y[i] * scale_factor, TNR3_MIN_VALUE, TNR3_MAX_VALUE);
		params->tnr3_vmem_params.sigma[j + TNR3_NUM_SEGMENTS] = clamp(yintercept_u[i] * scale_factor, TNR3_MIN_VALUE, TNR3_MAX_VALUE);
		params->tnr3_vmem_params.sigma[j + 2 * TNR3_NUM_SEGMENTS] = clamp(yintercept_v[i] * scale_factor, TNR3_MIN_VALUE, TNR3_MAX_VALUE);
	}

	params->use.tnr3_vmem_params = 1;
}

static void
ispTnr3DmemEncode(aic_config *config, ipu3_uapi_params *params)
{
	CLEAR(params->tnr3_dmem_params);

	params->tnr3_dmem_params.knee_y1 = config->tnr3_2500_config.knee_y[0];
	params->tnr3_dmem_params.knee_y2 = config->tnr3_2500_config.knee_y[1];
	params->tnr3_dmem_params.maxfb_y = config->tnr3_2500_config.maxfb_y;
	params->tnr3_dmem_params.maxfb_u = config->tnr3_2500_config.maxfb_u;
	params->tnr3_dmem_params.maxfb_v = config->tnr3_2500_config.maxfb_v;
	params->tnr3_dmem_params.round_adj_y = config->tnr3_2500_config.round_adj_y;
	params->tnr3_dmem_params.round_adj_u = config->tnr3_2500_config.round_adj_u;
	params->tnr3_dmem_params.round_adj_v = config->tnr3_2500_config.round_adj_v;
	params->tnr3_dmem_params.ref_buf_select = config->tnr3_2500_config.ref_buf_select;

	params->use.tnr3_dmem_params = 1;
}

void ParameterEncoder::encode(aic_config *config, ipu3_uapi_params *params)
{
	//ispAwbFrEncode(config, params); - an assert failure
	ispAeEncode(config, params);
	ispAwbEncode(config, params);
	ispAfEncode(config, params);
	ispLinVmemEncode(config, params);
	//ispGammaCtrlEncode(config, params); - segfault in KBL_AIC::run()
	ispCcmEncode(config, params);
	ispCscEncode(config, params);
	ispCdsEncode(config, params);
	ispDmEncode(config, params);
	ispShdEncode(config, params);
	ispIefdEncode(config, params);
	ispYdsEncode(config, params);
	ispYdsC0Encode(config, params);
	ispYds2Encode(config, params);
	ispChnrEncode(config, params);
	ispChnrC0Encode(config, params);
	ispYEeNrEncode(config, params);
	ispTccEncode(config, params);
	ispAnrEncode(config, params);
	ispBnrEncode(config, params);
	ispOBGEncode(config, params);
	ispBnrGreenDisparityEncode(config, params);
	ispXnr3Encode(config, params);
	ispXnr3VmemEncode(config, params);
	ispTnr3VmemEncode(config, params);
	ispTnr3DmemEncode(config, params);

	return;
}

} /* namespace libcamera */
