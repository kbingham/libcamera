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

#ifndef _SHD_PUBLIC_H_
#define _SHD_PUBLIC_H_

/** @file
* CSS-API header file for 2500/Skycam Shading correction (SHD) kernel/accelerator.
*/
//#include <type_support.h>

enum {
    SHD_MIN_GRID_WIDTH  = 8,
    SHD_MIN_GRID_HEIGHT = 8,
    SHD_MAX_GRID_WIDTH  = 73,
    SHD_MAX_GRID_HEIGHT = 55,
    SHD_MAX_GRID_SIZE   = SHD_MAX_GRID_WIDTH * SHD_MAX_GRID_HEIGHT
};

struct shd_public_config {
	/** description: enable shading correction fixed function processing
	* range:       [0 - disable,1 - enable] */
	unsigned int shd_enable;
	/** description: gain factor (aka 'gf')
	* range:       U2 [0,1,2] */
	unsigned int gain_factor;
};

/**
* Config struct for 2500/Skycam Shading correction (SHD) kernel/accelerator.
*
* general grid limitation:
* grid total size is larger than (or equal to) frame size
*/
struct shd_grid_public_config {
	/** description: number of grid vertices on the horizontal axis
	range:       [8..73] */
	unsigned int grid_width;
	/** description: number of grid vertices on the vertical axis
	range:       [8..56] */
	unsigned int grid_height;
	/** description: log2 of each grid cell width
	range:       [3..7] (cell width values [8,16,32,64,128]) */
	unsigned int block_width;
	/** description: log2 of each grid cell height
	range:       [3..7] (cell height values [8,16,32,64,128]) */
	unsigned int block_height;
	/** description: horizontal offset of the grid in relation to the frame
	range:       [-4096..0] (grid starts left to the frame start)
	other limitations: x_start%2=0 */
	signed   int x_start;
	/** description: vertical offset of the grid in relation to the frame
	range:       [-4096..0] (grid starts above the frame start)
	other limitations: y_start%2=0 */
	signed   int y_start;
};

struct shd_black_level_public_config {
	/** description: black level bias value
	range:       S12 [-2048..2047] */
	signed int bl_R;
	/** description: black level bias value
	range:       S12 [-2048..2047] */
	signed int bl_Gr;
	/** description: black level bias value
	range:       S12 [-2048..2047] */
	signed int bl_Gb;
	/** description: black level bias value
	range:       S12 [-2048..2047] */
	signed int bl_B;
	/** description: shift-left value for normaliztion factor (aka 'nf')
	range:       [0,1,3,5] */
	unsigned int normalization_shift;
};

struct shd_luts_public_config {
	/**
	 * description: shading factor
	 * range:       U12
	 * note 1: refer to shd HAS doc for internal interpretation
	 * note 2: range is not enforced (too much overhead)
	 */
	unsigned short R[SHD_MAX_GRID_SIZE];
	/**
	 * description: shading factor
	 * range:       U12
	 * note 1: refer to shd HAS doc for internal interpretation
	 * note 2: range is not enforced (too much overhead)
	 */
	unsigned short Gr[SHD_MAX_GRID_SIZE];
	/**
	 * description: shading factor
	 * range:       U12
	 * note 1: refer to shd HAS doc for internal interpretation
	 * note 2: range is not enforced (too much overhead)
	 */
	unsigned short Gb[SHD_MAX_GRID_SIZE];
	/**
	 * description: shading factor
	 * range:       U12
	 * note 1: refer to shd HAS doc for internal interpretation
	 * note 2: range is not enforced (too much overhead)
	 */
	unsigned short B[SHD_MAX_GRID_SIZE];
};

// OPEN - what about the enable?
struct shd_kernel_config {

    struct shd_public_config general;

    struct shd_grid_public_config grid;

    struct shd_black_level_public_config black_level;

    struct shd_luts_public_config luts;
};

struct ia_css_2500_shd_kernel_config {
    struct shd_kernel_config shd;
};

/** @brief Print SHD public configuration
 *
 * @param	cfg	The pointer to configuration data
 * @return	None
 *
 * Print SHD public configuration.
 */
void ia_css_shd_public_cfg_dump(const struct ia_css_2500_shd_kernel_config *cfg);

/** @brief Compare two SHD public configurations
 *
 * @param	cfg1	The pointer to first configuration data
 * @param	cfg2	The pointer to second configuration data
 * @param	cfg_dump	Configurations are printed in case of
 *   			   mismatch
 * @return	true - match, false - not match
 *
 * Compare two SHD public configurations
 */
bool ia_css_shd_public_cfg_compare(
	const struct ia_css_2500_shd_kernel_config *cfg1,
	const struct ia_css_2500_shd_kernel_config *cfg2,
	bool cfg_dump);

#endif // _SHD_PUBLIC_H_
