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

#ifndef _YUVP1_PUBLIC_H_
#define _YUVP1_PUBLIC_H_

/** @file
* CSS-API header file for 2500/Skycam B0 specific noice reduction (YUV_P1 component) kernel/accelerator.
*/
//#include <type_support.h>

/* All structs in this file will be visible to the CSS API User.
 * The User will be able to set each one of the config params separately by
 * specifying a different config type (see sh_config_type in sh_css.h).
 * If one of those config structs is not set explicitly, a default value will
 * be use instead.
 */

#include "yuvp1_common_public.h"

struct yuvp1_y_ee_nr_lpf_public_config {
	unsigned char a_diag;
	unsigned char a_periph;
	unsigned char a_cent;
	unsigned char y_ee_nr_en;
};

struct yuvp1_y_ee_nr_sense_public_config {

	unsigned short edge_sense_0;
	unsigned short delta_edge_sense;
	unsigned short corner_sense_0;
	unsigned short delta_corner_sense;

};

struct yuvp1_y_ee_nr_gain_public_config {

	unsigned char gain_pos_0;
	unsigned char delta_gain_posi;
	unsigned char gain_neg_0;
	unsigned char delta_gain_neg;

};

struct yuvp1_y_ee_nr_clip_public_config {

	unsigned char clip_pos_0;
	unsigned char delta_clip_posi;
	unsigned char clip_neg_0;
	unsigned char delta_clip_neg;

};

struct yuvp1_y_ee_nr_frng_public_config {

	unsigned char gain_exp;
	unsigned short min_edge;
	unsigned char lin_seg_param;
	unsigned char t1;
	unsigned char t2;
};

struct yuvp1_y_ee_nr_diag_public_config {

	unsigned char diag_disc_g;
	unsigned char hvw_hor;
	unsigned char dw_hor;
	unsigned char hvw_diag;
	unsigned char dw_diag;
};

struct yuvp1_y_ee_nr_fc_coring_public_config {

	unsigned short pos_0;
	unsigned short pos_delta;
	unsigned short neg_0;
	unsigned short neg_delta;

};

struct yuvp1_y_ee_nr_public_config {

	struct yuvp1_y_ee_nr_lpf_public_config lpf;
	struct yuvp1_y_ee_nr_sense_public_config sense;
	struct yuvp1_y_ee_nr_gain_public_config gain;
	struct yuvp1_y_ee_nr_clip_public_config clip;
	struct yuvp1_y_ee_nr_frng_public_config frng;
	struct yuvp1_y_ee_nr_diag_public_config diag;
	struct yuvp1_y_ee_nr_fc_coring_public_config fc_coring;

};

struct ia_css_2500_yuvp1_b0_kernel_config {
	struct yuvp1_y_ee_nr_public_config y_ee_nr;
	struct yuvp1_yds_public_config yds;
	struct yuvp1_chnr_public_config chnr;
};

/** @brief Print YUVP1_B0 public configuration
 *
 * @param	cfg	The pointer to configuration data
 * @return	None
 *
 * Print YUVP1_B0 public configuration.
 */
void ia_css_yuvp1_b0_public_cfg_dump(
	const struct ia_css_2500_yuvp1_b0_kernel_config *cfg);

/** @brief Compare two YUVP1_B0 public configurations
 *
 * @param	cfg1	The pointer to first configuration data
 * @param	cfg2	The pointer to second configuration data
 * @param	cfg_dump	Configurations are printed in case of
 *   			   mismatch
 * @return	true - match, false - not match
 *
 * Compare two YUVP1_B0 public configurations
 */
bool ia_css_yuvp1_b0_public_cfg_compare(
	const struct ia_css_2500_yuvp1_b0_kernel_config *cfg1,
	const struct ia_css_2500_yuvp1_b0_kernel_config *cfg2,
	bool cfg_dump);

#endif
