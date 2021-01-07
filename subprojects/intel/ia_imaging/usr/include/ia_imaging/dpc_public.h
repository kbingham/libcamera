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

/** @file
* CSS-API header file for 2500/Skycam Defect Pixel Correction (DPC) kernel/accelerator.
*/
//#include <type_support.h>

#ifndef DPC_PUBLIC_H_
#define DPC_PUBLIC_H_

/** DPC can correct due to hardware limitations up to 192 pixels per 50 lines.
  *  So for an input height of 3840 lines this results into 77 sets of 50 lines.
  *  As a result the maximum number of dead pixels is 192*77.
  */
#define DPC_MAX_NUMBER_OF_DP 14784

// lut entry
typedef struct dpc_public_lut_elem {

	unsigned int nghbr_sts:5;
	unsigned int skip:1;
	unsigned int nghbr_order:1;
	unsigned int column:13;
	unsigned int row_pair_delta:4;
	unsigned int spare0:8;

} dpc_public_lut_elem_t;


typedef struct dpc_public_lut {

	dpc_public_lut_elem_t elems[DPC_MAX_NUMBER_OF_DP];

} dpc_public_lut_t;

struct dpc_public_config {

	unsigned int grad_threshold;
	unsigned int num_of_dp_gr;
	unsigned int num_of_dp_bg;
	dpc_public_lut_t lut_gr;
	dpc_public_lut_t lut_bg;

};

struct ia_css_2500_dpc_kernel_config {

	struct dpc_public_config dpc;

};

/** @brief Print DPC public configuration
 *
 * @param	cfg	The pointer to configuration data
 * @return	None
 *
 * Print DPC public configuration.
 */
void ia_css_dpc_public_cfg_dump(
	const struct ia_css_2500_dpc_kernel_config *cfg);

/** @brief Compare two DPC public configurations
 *
 * @param	cfg1	The pointer to first configuration data
 * @param	cfg2	The pointer to second configuration data
 * @param	cfg_dump	Configurations are printed in case of
 *   			   mismatch
 * @return	true - match, false - not match
 *
 * Compare two DPC public configurations
 */
bool ia_css_dpc_public_cfg_compare(
	const struct ia_css_2500_dpc_kernel_config *cfg1,
	const struct ia_css_2500_dpc_kernel_config *cfg2,
	bool cfg_dump);

#endif /* DPC_PUBLIC_H_ */
