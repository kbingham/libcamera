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

#ifndef _BNR_PUBLIC_H_
#define _BNR_PUBLIC_H_

/** @file
* CSS-API header file for 2500/Skycam Bayer Noise Removal (BNR) kernel/accelerator.
*/
//#include <type_support.h>

// All structs in this file will be visible to the CSS API User.
// The User will be able to set each one of the config params separately by
// specifying a different config type (see sh_config_type in sh_css.h).
// If one of those config structs is not set explicitly, a default value will
// be use instead.

typedef struct {
   unsigned short gr;
   unsigned short r;
   unsigned short b;
   unsigned short gb;
}bnr_public_config_wb_gains_config_t;

typedef struct {
   unsigned char gr;
   unsigned char r;
   unsigned char b;
   unsigned char gb;
}bnr_public_config_wb_gains_thr_config_t;

typedef struct {
   unsigned short cf;
   unsigned char cg;
   unsigned char ci;
   unsigned char r_nf;
}bnr_public_config_thr_coeffs_config_t;

typedef struct {
   unsigned char gr;
   unsigned char r;
   unsigned char b;
   unsigned char gb;
}bnr_public_config_thr_ctrl_shd_config_t;

typedef struct {
   short x_reset;
   short y_reset;
   unsigned int x_sqr_reset;
   unsigned int y_sqr_reset;
}bnr_public_config_opt_center_config_t;

typedef struct {
   unsigned char bp_thr_gain;
   unsigned char defect_mode;
   unsigned int bp_gain;
   unsigned int w0_coeff;
   unsigned int w1_coeff;
}bnr_public_config_bp_ctrl_config_t;


typedef struct {
	unsigned char alpha;	/**< Weight of central element of
				     smoothing filter
				     range: [0..0xF]*/
	unsigned char beta;	/**< Weight of peripheral elements of
				     smoothing filter
				     range: [0..0xF]*/
	unsigned char gamma;	/**< Weight of diagonal elements of
				     smoothing filter
				     range: [0..0xF]*/
	unsigned short max_inf;	/**< Maximum increase of peripheral
				     or diagonal element influence
				     relative to the  predefined value
				     range: [0x5..0xA]*/
	unsigned char gd_en;	/**< Greenn Disparity ff enable control
				     range: [0 - disable, 1 - enable] */
	unsigned char bpc_en;	/**< Bad Pixel Correction ff enable control
				     range: [0 - disable, 1 - enable] */
	unsigned char bnr_en;	/**< Bayer Noise Removal ff enable control
				     range: [0 - disable, 1 - enable] */
}bnr_public_config_dn_detect_ctrl_config_t;

#define BNR_LUT_SIZE 32
typedef struct {
   unsigned char values[BNR_LUT_SIZE];
} bnr_public_config_lut_config_t;

typedef struct bnr_public_config_green_disparity {
    unsigned int GD_Red;
    unsigned int GD_Green;
    unsigned int GD_Blue;
    unsigned int GD_Black;
    unsigned int GD_Shading;
    unsigned int GD_Support;
    unsigned int GD_Clip;
    unsigned int GD_Central_Weight;
} bnr_public_config_lut_config_green_disparity_t;

struct bnr_public_config
{
    bnr_public_config_wb_gains_config_t            wb_gains;
    bnr_public_config_wb_gains_thr_config_t        wb_gains_thr;
    bnr_public_config_thr_coeffs_config_t          thr_coeffs;
    bnr_public_config_thr_ctrl_shd_config_t        thr_ctrl_shd;
    bnr_public_config_opt_center_config_t          opt_center;
    bnr_public_config_bp_ctrl_config_t             bp_ctrl;
    bnr_public_config_dn_detect_ctrl_config_t      dn_detect_ctrl;
    bnr_public_config_lut_config_t                 lut;
};

struct ia_css_2500_bnr_kernel_config
{
    struct bnr_public_config bnr;
    bnr_public_config_lut_config_green_disparity_t green_disparity;
};

/** @brief Print BNR public configuration
 *
 * @param	cfg	The pointer to configuration data
 * @return	None
 *
 * Print BNR public configuration.
 */
void ia_css_bnr_public_cfg_dump(
	const struct ia_css_2500_bnr_kernel_config *cfg);

/** @brief Compare two BNR public configurations
 *
 * @param	cfg1	The pointer to first configuration data
 * @param	cfg2	The pointer to second configuration data
 * @param	cfg_dump	Configurations are printed in case of
 *   			   mismatch
 * @return	true - match, false - not match
 *
 * Compare two BNR public configurations
 */
bool ia_css_bnr_public_cfg_compare(
	const struct ia_css_2500_bnr_kernel_config *cfg1,
	const struct ia_css_2500_bnr_kernel_config *cfg2,
	bool cfg_dump);

#endif // _BNR_PUBLIC_H_
