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

/* Imported directly from CommonUtilMacros.h */
#ifndef MEMCPY_S
#define MEMCPY_S(dest, dmax, src, smax) memcpy((dest), (src), std::min((size_t)(dmax), (size_t)(smax)))
#endif
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLEAR(x) memset(&(x), 0, sizeof(x))

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

	return;
}

} /* namespace libcamera */
