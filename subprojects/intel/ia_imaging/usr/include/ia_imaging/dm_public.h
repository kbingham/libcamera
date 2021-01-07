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

#ifndef _DM_PUBLIC_H_
#define _DM_PUBLIC_H_

/** @file
* CSS-API header file for 2500/Skycam Demosaic (DM) kernel/accelerator.
*/
//#include <type_support.h>

struct dm_public_config {
    /** Demosaic enable flag */
    unsigned char dm_en;
    /** Checker Artifact Removal enable */
    unsigned char ch_ar_en;
    /** false color correction enable */
    unsigned char fcc_en;
    /** sharpening coefficient for green calculation [0..31] */
    unsigned char gamma_sc;
    /** control param for weights of Chroma Homogeneity metric [0..31] */
    unsigned char lc_ctrl;
    /** Control param for Checker artifact removal [0..31] */
    unsigned char cr_param1;
    /** Control param for Checker artifact removal [0..31] */
    unsigned char cr_param2;
    /** False color correction control [0..31] */
    unsigned char coring_param;
};

struct ia_css_2500_dm_kernel_config {
    struct dm_public_config dm;
};

/** @brief Print DM public configuration
 *
 * @param	cfg	The pointer to configuration data
 * @return	None
 *
 * Print DM public configuration.
 */
void ia_css_dm_public_cfg_dump(
	const struct ia_css_2500_dm_kernel_config *cfg);

/** @brief Compare two DM public configurations
 *
 * @param	cfg1	The pointer to first configuration data
 * @param	cfg2	The pointer to second configuration data
 * @param	cfg_dump	Configurations are printed in case of
 *   			   mismatch
 * @return	true - match, false - not match
 *
 * Compare two DM public configurations
 */
bool ia_css_dm_public_cfg_compare(
	const struct ia_css_2500_dm_kernel_config *cfg1,
	const struct ia_css_2500_dm_kernel_config *cfg2,
	bool cfg_dump);

#endif // _DM_PUBLIC_H_
