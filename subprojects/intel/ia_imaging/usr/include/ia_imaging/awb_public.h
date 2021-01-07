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

#ifndef _AWB_PUBLIC_H_
#define _AWB_PUBLIC_H_

/** @file
* CSS-API header file for 2500/Skycam Auto White Balance (AWB) kernel/accelerator.
*/
//#include <type_support.h>

// All structs in this file will be visible to the CSS APU User.
// The User will be able to set each one of the config params separately by
// specifying a different config type (see sh_config_type in sh_css.h).
// If one of those config structs is not set explicitly, a default value will
// be use instead.

#define AWB_MIN_GRID_WIDTH   16
#define AWB_MIN_GRID_HEIGHT  16
#define AWB_MAX_GRID_WIDTH   80
#define AWB_MAX_GRID_HEIGHT  60

struct awb_public_config_grid_config{
	unsigned char grid_width;        /**< number of horizontal grid cells */
	unsigned char grid_height;       /**< number of vertical grid cells */
	unsigned char grid_block_width;  /**< log2 the width of each cell (8,16,32,64) */
	unsigned char grid_block_height; /**< log2 the height of each cell (8,16,32,64) */
	unsigned short grid_x_start;     /**< x top left corner of the grid */
	unsigned short grid_y_start;     /**< y top left corner of the grid */
};

struct awb_public_config {
    unsigned short rgbs_Gr_threshold;
    unsigned short rgbs_R_threshold;
    unsigned short rgbs_B_threshold;
    unsigned short rgbs_Gb_threshold;
    unsigned char rgbs_en;
    unsigned char rgbs_incl_sat;
    struct awb_public_config_grid_config grid;
};

struct ia_css_2500_awb_kernel_config{
    struct awb_public_config awb;
};



// awb public meta-data types
typedef struct awb_public_meta{
    int dummy;
} awb_public_meta_t;

typedef struct awb_public_set_item{
    unsigned char Gr_avg;
    unsigned char R_avg;
    unsigned char B_avg;
    unsigned char Gb_avg;
    unsigned char sat_ratio;
    unsigned char padding0; /**< Added the padding so that the public matches that private */
    unsigned char padding1; /**< Added the padding so that the public matches that private */
    unsigned char padding2; /**< Added the padding so that the public matches that private */
} awb_public_set_item_t;


#define AWB_PUBLIC_NUM_OF_ITEMS_IN_SET 160
/** Based on max grid height + Spare for bubbles */
#define AWB_PUBLIC_NUM_OF_SETS_IN_BUFFER (60 + 20)

typedef struct awb_public_raw_buffer{
    awb_public_set_item_t rgb_table[AWB_PUBLIC_NUM_OF_SETS_IN_BUFFER*AWB_PUBLIC_NUM_OF_ITEMS_IN_SET];
} awb_public_raw_buffer_t;

/** @brief Print AWB public configuration
 *
 * @param	cfg	The pointer to configuration data
 * @return	None
 *
 * Print AWB public configuration.
 */
void ia_css_awb_public_cfg_dump(const struct ia_css_2500_awb_kernel_config *cfg);

/** @brief Compare two AWB public configurations
 *
 * @param	cfg1	The pointer to first configuration data
 * @param	cfg2	The pointer to second configuration data
 * @param	cfg_dump	Configurations are printed in case of
 *   			   mismatch
 * @return	true - match, false - not match
 *
 * Compare two AWB public configurations
 */
bool ia_css_awb_public_cfg_compare(
	const struct ia_css_2500_awb_kernel_config *cfg1,
	const struct ia_css_2500_awb_kernel_config *cfg2,
	bool cfg_dump);

#endif // _AWB_PUBLIC_H_
