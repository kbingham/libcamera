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

#ifndef _AE_PUBLIC_H_
#define _AE_PUBLIC_H_

/** @file
* CSS-API header file for 2500/Skycam Auto Exposure (AE) kernel/accelerator.
*/
//#include <type_support.h>

typedef struct {
	unsigned short gain_GR;
	unsigned short gain_R;
	unsigned short gain_B;
	unsigned short gain_GB;
} ae_public_config_ccm_wb_coeffs_t;


#define AE_CCM_NUM_OF_MAT_COEFFS 16

typedef struct {
	signed short coeffs[AE_CCM_NUM_OF_MAT_COEFFS];
} ae_public_config_ccm_mat_coeffs_t;


#define AE_NUM_OF_WEIGHTS (32*24)

typedef struct ae_public_config_weights {
    unsigned char val[AE_NUM_OF_WEIGHTS];
} ae_public_config_weights_t;


struct ae_public_config_grid_config{
	unsigned char  grid_width;   /**< number of horizontal grid cells */
	unsigned char  grid_height;  /**< number of vertical grid cells */
	unsigned char  block_width;  /**< log2 the width of each cell (8,16,32,64,128) */
	unsigned char  block_height; /**< log2 the height of each cell (8,16,32,64,128) */
	unsigned short x_start;      /**< x top left corner of the grid */
	unsigned short y_start;      /**< y top left corner of the grid */
	unsigned char ae_en;         /**< 0 - ae ff does not write to meta-data array,
					  1 - ae ff writes to meta-data array */
};

typedef struct ae_public_config_ccm_coeffs {
	ae_public_config_ccm_wb_coeffs_t  wb_coeffs;
	ae_public_config_ccm_mat_coeffs_t mat_coeffs;
} ae_public_config_ccm_coeffs_t;

struct ae_public_config {
	ae_public_config_ccm_coeffs_t	ae_ccm;
	ae_public_config_weights_t		ae_weights;
	struct ae_public_config_grid_config ae_grid_config;
};

struct ia_css_2500_ae_kernel_config {
	struct ae_public_config ae;
};



/* ae public meta-data types */
#define AE_NUM_OF_HIST_BINS 256
typedef struct {
	unsigned int vals[AE_NUM_OF_HIST_BINS];
} ae_public_color_hist_t;

typedef struct ae_public_raw_buffer {
	ae_public_color_hist_t hist_R;
	ae_public_color_hist_t hist_G;
	ae_public_color_hist_t hist_B;
	ae_public_color_hist_t hist_Y;
} ae_public_raw_buffer_t; 

/** @brief Print AE public configuration
 *
 * @param	cfg	The pointer to configuration data
 * @return	None
 *
 * Print AE public configuration.
 */
void ia_css_ae_public_cfg_dump(
	const struct ia_css_2500_ae_kernel_config *cfg);

/** @brief Compare two AE public configurations
 *
 * @param	cfg1	The pointer to first configuration data
 * @param	cfg2	The pointer to second configuration data
 * @param	cfg_dump	Configurations are printed in case of
 *   			   mismatch
 * @return	true - match, false - not match
 *
 * Compare two AE public configurations
 */
bool ia_css_ae_public_cfg_compare(
	const struct ia_css_2500_ae_kernel_config *cfg1,
	const struct ia_css_2500_ae_kernel_config *cfg2,
	bool cfg_dump);
#endif /* _AE_PUBLIC_H_ */
