/*
 * Copyright (C) 2015 - 2017 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _YUVP1_C0_PUBLIC_H_
#define _YUVP1_C0_PUBLIC_H_

/** @file
* CSS-API header file for 2500/Skycam C0 specific noice reduction (YUV_P1 component) kernel/accelerator.
*/
//#include <type_support.h>

/* All structs in this file will be visible to the CSS API User.
 * The User will be able to set each one of the config params separately by
 * specifying a different config type (see sh_config_type in sh_css.h).
 * If one of those config structs is not set explicitly, a default value will
 * be use instead.
 */

#include "yuvp1_common_public.h"

/*******************************************************************/
/*                      IEFD Config Units                          */
/*******************************************************************/
struct cux2 {
	short x[2];
	short a[1];
	short b[1];
};

struct cux4 {
	short x[4];
	short a[3];
	short b[3];
};

struct cux6 {
	short x[6];
	short a[5];
	short b[5];
};


/**
 * \brief IEFD's Configuration Units
 * \details Each configuration unit define weights of different calculations.
 */
struct public_cfg_units {
	struct cux2 cu_1;       /**< calculates weight for blending directed and non-directed denoise elements */
	struct cux6 cu_ed;      /**< calculates weight for mean3x3 */
	struct cux2 cu_3;       /**< calculates power wdn of directed denoise element */
	struct cux2 cu_5;       /**< calculates power of non-directed denoise element apply */
	struct cux4 cu_6;       /**< calculates power  of non-directed sharpening element apply */
	struct cux2 cu_7;       /**< calculates weight wos_directed for over-sharpening limit calculation */
	struct cux4 cu_unsharp; /**< calculates unsharp mask for unsharp-blend calculation */
	struct cux6 cu_radial;  /**< calculates various radial parameters */
	struct cux2 cu_vssnlm;  /**< used to apply vssnlm filter only on edges */
};

/*******************************************************************/
/*                      IEFD Config & Cotrol                       */
/*******************************************************************/
/**
 * \brief IEFD's Configuration
 * \details Holds general IEFD configurations
 */
struct yuvp1_iefd_config_public_config {

	unsigned char horver_diag_coeff;/**< Coefficient that compensates for different distance for
										vertical/horizontal and diagonal gradient calculation (~1/sqrt(2)) */

	unsigned char clamp_stitch; /**< Slope to stitch between clamped and unclamped edge values */

	unsigned char direct_metric_update; /**< Update coefficient for direction metric */

	unsigned char ed_horver_diag_coeff; /**< Radial Coefficient that compensates for different distance for
											vertical/horizontal and diagonal gradient calculation (~1/sqrt(2)) */
};

/**
 * \brief IEFD's Control
 * \details Controls algorithm's parts
 */
struct yuvp1_iefd_control_public_config {
	unsigned char iefd_en;
	unsigned char iefd_denoise_en;
	unsigned char iefd_dir_en;     /**< Enable smooth best direction with second best */
	unsigned char iefd_radial_en;  /**< Enable update radial dependent parameters */
	unsigned char iefd_vssnlm_en;  /**< Enable VSSNLM output filter */
};

/*******************************************************************/
/*                      Sharpening Configuration                   */
/*******************************************************************/

/**
 * \brief Sharpening Limit Configuration
 */
struct sharp_public_config {

	unsigned short nega_lmt_txt; /**< Sharpening limit for negative overshoots for texture */
	unsigned short posi_lmt_txt; /**< Sharpening limit for positive overshoots for texture */
	unsigned short nega_lmt_dir; /**< Sharpening limit for negative overshoots for direction (edge) */
	unsigned short posi_lmt_dir; /**< Sharpening limit for positive overshoots for direction (edge) */
};

/**
 * \brief Direct/Non-Direct parameters
 */
struct far_w_public_config {
	unsigned char dir_shrp; /**< Weight of wide direct sharpening */
	unsigned char dir_dns; /**< Weight of wide direct denoising */
	unsigned char ndir_dns_powr; /**< Power of non-direct denoising  */
};

/**
 * \brief UnSharp Configuration
 */
struct unsharp_cfg_public_config {
	unsigned char unsharp_weight; /**< Unsharp Mask blending weight. 0 - disabled 64- use only Unsharp */
	unsigned short unsharp_amount; /**< Unsharp Mask amount */
};

/**
 * \brief Sharpening Configuration
 */
struct yuvp1_iefd_shrp_cfg_public_config {
	struct sharp_public_config cfg;
	struct far_w_public_config far_w;
	struct unsharp_cfg_public_config unshrp_cfg;

};

/*******************************************************************/
/*                      Unsharp mask Configuration                 */
/*******************************************************************/


struct unsharp_coef0_public_config {
	unsigned short c00; /* Coeff11 */
	unsigned short c01; /* Coeff12 */
	unsigned short c02; /* Coeff13 */
};


struct unsharp_coef1_public_config {
	unsigned short c11; /* Coeff22 */
	unsigned short c12; /* Coeff23 */
	unsigned short c22; /* Coeff33 */
};

/**
 * \brief Unsharp Mask Filter Coefficients
 */
struct yuvp1_iefd_unshrp_cfg_public_config {
	struct unsharp_coef0_public_config unsharp_coef0;
	struct unsharp_coef1_public_config unsharp_coef1;
};

/*******************************************************************/
/*                      Radius calculation Configuration           */
/*******************************************************************/

/**
 * \brief Radial Reset Parameters
 */
struct radial_reset {

	short x; /**< Radial centre of X */
	short y; /**< Radial centre of Y */
	unsigned int x2; /**< X^2 */
	unsigned int y2; /**< Y^2 */
};

/**
 * \brief Radial Normalization parameters
 * \details Used to normalize radius
 */
struct radial_cfg_public_config {
	unsigned char rad_nf; /**< Radial norm factor */
	unsigned char rad_inv_r2; /**< Radial R2 normalized */
};

/**
 * \brief Radial Direct/non-Direct parameters
 */
struct rad_far_w_public_config {
	unsigned char rad_dir_far_sharp_w; /**< Radial weight of wide direct sharpening */
	unsigned char rad_dir_far_dns_w; /**< Radial weight of wide direct denoising */
	unsigned char rad_ndir_far_dns_power; /**< Radial Power of non-direct denoising */
};

/**
 * \brief Configuration Units Power
 * \details Determines the power and radial power of various configuration units
 */
struct cu_cfg0_public_config {
	unsigned char cu6_pow; /**< Power of CU6 (power of non-direct sharpening) */
	unsigned char cu_unsharp_pow; /**< Power of CUUnsharp (power of unsharp mask) */
	unsigned char rad_cu6_pow; /**< Radial/corner CU6 Directed sharpening power */
	unsigned char rad_cu_unsharp_pow; /**< Radial power of CUUnsharp (power of unsharp mask) */
};

/**
 * \brief Configuration Units Points Radial Configuration
 */
struct cu_cfg1_public_config {
	unsigned short rad_cu6_x1; /**< X1 point of configuration unit 6. must be in the range (X0 of CU6, X2 of CU6)*/
	unsigned short rad_cu_unsharp_x1; /**< X1 point for configuration unit Unsharp for radial/corner point
										must be in the range (X0 of CU_unsharp, X2 of CU_unsharp)*/
};

/**
 * \brief Radial Configuration
 */
struct yuvp1_iefd_rad_cfg_public_config {
	struct radial_reset reset;
	struct radial_cfg_public_config cfg;
	struct rad_far_w_public_config rad_far_w;
	struct cu_cfg0_public_config cu_cfg0;
	struct cu_cfg1_public_config cu_cfg1;
};

/*******************************************************************/
/*                      VSSNLM  Configuration                      */
/*******************************************************************/

/**
 * \brief VSSNLM X LUT
 */
struct vss_lut_x_public_config {
	unsigned char vs_x0;
	unsigned char vs_x1;
	unsigned char vs_x2;
};

/**
 * \brief VSSNLM Y LUT
 */
struct vss_lut_y_public_config {
	unsigned char vs_y1;
	unsigned char vs_y2;
	unsigned char vs_y3;
};

/**
 * \brief VSSNLM LUT Configuration
 */
struct yuvp1_iefd_vssnlm_cfg_public_config {
	struct vss_lut_x_public_config vss_lut_x;
	struct vss_lut_y_public_config vss_lut_y;
};

/**
 * \brief IEFD COnfiguration
 */
struct yuvp1_iefd_public_config {
	struct public_cfg_units cfg_units;
	struct yuvp1_iefd_config_public_config config;
	struct yuvp1_iefd_control_public_config control;
	struct yuvp1_iefd_shrp_cfg_public_config sharp;
	struct yuvp1_iefd_unshrp_cfg_public_config unsharp;
	struct yuvp1_iefd_rad_cfg_public_config rad;
	struct yuvp1_iefd_vssnlm_cfg_public_config vsslnm;
};

/**
 * \brief YUVP1 C0 Configuration
 * \details define full YUVP1 accelerator configuration for C0 (IEFD ff replaces B0's Y_EE_NR)
 */
struct ia_css_2500_yuvp1_c0_kernel_config {

	struct yuvp1_iefd_public_config iefd;
	struct yuvp1_yds_public_config yds_c0;
	struct yuvp1_chnr_public_config chnr_c0;
};

/** @brief Print YUVP1_C0 public configuration
 *
 * @param	cfg	The pointer to configuration data
 * @return	None
 *
 * Print YUVP1_C0 public configuration.
 */
void ia_css_yuvp1_c0_public_cfg_dump(
	const struct ia_css_2500_yuvp1_c0_kernel_config *cfg);

/** @brief Compare two YUVP1_C0 public configurations
 *
 * @param	cfg1	The pointer to first configuration data
 * @param	cfg2	The pointer to second configuration data
 * @param	cfg_dump	Configurations are printed in case of
 *   			   mismatch
 * @return	true - match, false - not match
 *
 * Compare two YUVP1_C0 public configurations
 */
bool ia_css_yuvp1_c0_public_cfg_compare(
	const struct ia_css_2500_yuvp1_c0_kernel_config *cfg1,
	const struct ia_css_2500_yuvp1_c0_kernel_config *cfg2,
	bool cfg_dump);

#endif
