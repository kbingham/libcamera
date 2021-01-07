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

#ifndef _YUVP2_PUBLIC_H_
#define _YUVP2_PUBLIC_H_

/** @file
* CSS-API header file for 2500/Skycam color enhancement (YUV_P2 component) kernel/accelerator.
*/
//#include <type_support.h>

/*
 * All structs in this file will be visible to the CSS API User.
 * The User will be able to set each one of the config params separately by
 * specifying a different config type (see sh_config_type in sh_css.h).
 * If one of those config structs is not set explicitly, a default value will
 * be use instead.
 */

#include "yuvp2_common_defs.h"

/******************
*	Y-TM
*******************/

struct yuvp2_ytm_public_config {
	unsigned short entries[YUVP2_YTM_LUT_NUM_OF_ENTRIES];
	unsigned int   ytm_en;
};

/******************
*	YDS
*******************/
struct yuvp2_yds_public_config {

	unsigned char c00;
	unsigned char c01;
	unsigned char c02;
	unsigned char c03;
	unsigned char c10;
	unsigned char c11;
	unsigned char c12;
	unsigned char c13;
	unsigned char norm_factor;
	unsigned char bin_output;

};

/*******************
*	TCC
********************/

struct yuvp2_tcc_gen_control_public_config {
	unsigned char blend_shift;
	unsigned char gain_according_to_y_only;
	signed char gamma;
	signed char delta;

};

struct yuvp2_tcc_macc_elem_public_config {
	signed short A;
	signed short B;
	signed short C;
	signed short D;
};

struct yuvp2_tcc_macc_table_public_config {
	struct yuvp2_tcc_macc_elem_public_config entries[YUVP2_TCC_NUM_OF_MACC_TABLE_ELEMENTS];
};


struct yuvp2_tcc_inv_y_lut_public_config {
	unsigned short entries[YUVP2_TCC_NUM_OF_INV_Y_LUT_ELEMENTS];
};


struct yuvp2_tcc_gain_pcwl_lut_public_config {
	unsigned short entries[YUVP2_TCC_NUM_OF_GAIN_PCWL_LUT_ELEMENTS];
};


struct yuvp2_tcc_r_sqr_lut_public_config {
	unsigned short entries[YUVP2_TCC_NUM_OF_R_SQR_LUT_ELEMENTS];
};

struct yuvp2_tcc_public_config {
	struct yuvp2_tcc_gen_control_public_config   gen_control;
	struct yuvp2_tcc_macc_table_public_config    macc_table;
	struct yuvp2_tcc_inv_y_lut_public_config     inv_y_lut;
	struct yuvp2_tcc_gain_pcwl_lut_public_config gain_pcwl;
	struct yuvp2_tcc_r_sqr_lut_public_config     r_sqr_lut;
};


/****************************
*	YUVP2 combined
****************************/

struct ia_css_2500_yuvp2_kernel_config {

	struct yuvp2_ytm_public_config ytm;
	struct yuvp2_yds_public_config yds2;
	struct yuvp2_tcc_public_config tcc;
};

/** @brief Print YUVP2 public configuration
 *
 * @param	cfg	The pointer to configuration data
 * @return	None
 *
 * Print YUVP2 public configuration.
 */
void ia_css_yuvp2_public_cfg_dump(
	const struct ia_css_2500_yuvp2_kernel_config *cfg);

/** @brief Compare two YUVP2 public configurations
 *
 * @param	cfg1	The pointer to first configuration data
 * @param	cfg2	The pointer to second configuration data
 * @param	cfg_dump	Configurations are printed in case of
 *   			   mismatch
 * @return	true - match, false - not match
 *
 * Compare two YUVP2 public configurations
 */
bool ia_css_yuvp2_public_cfg_compare(
	const struct ia_css_2500_yuvp2_kernel_config *cfg1,
	const struct ia_css_2500_yuvp2_kernel_config *cfg2,
	bool cfg_dump);

#endif
