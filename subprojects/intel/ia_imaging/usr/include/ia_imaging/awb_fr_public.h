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

#ifndef _AWB_FR_PUBLIC_H_
#define _AWB_FR_PUBLIC_H_

/** @file
* CSS-API header file for 2500/Skycam Auto White Balance Filter Response (AWB) kernel/accelerator.
*/
//#include <type_support.h>

/* All structs in this file will be visible to the CSS API User.
 * The User will be able to set each one of the config params separately by
 * specifying a different config type (see sh_config_type in sh_css.h).
 * If one of those config structs is not set explicitly, a default value will
 * be used instead.
 */

#define AWB_FR_MIN_GRID_WIDTH   16
#define AWB_FR_MIN_GRID_HEIGHT  16
#define AWB_FR_MAX_GRID_WIDTH   32
#define AWB_FR_MAX_GRID_HEIGHT  24
#define AWB_FR_MIN_BLOCK_WIDTH  3
#define AWB_FR_MIN_BLOCK_HEIGHT 3
#define AWB_FR_MAX_BLOCK_WIDTH  6 /**< HSD1403986067 bug in c0, restrict max block width to 6 instead of 7 */
#define AWB_FR_MAX_BLOCK_HEIGHT 6 /**< HSD1403986067 bug in c0, restrict max block height to 6 instead of 7 */

typedef struct awb_fr_public_coeffs_config {
    unsigned char A1;		/**< default value 0 */
    unsigned char A2;		/**< default value 0 */
    unsigned char A3;		/**< default value 0 */
    unsigned char A4;		/**< default value 0 */
    unsigned char A5;		/**< default value 0 */
    unsigned char A6;		/**< default value 128 */
    unsigned int  sign_vec;	/**< default value 0 */
} awb_fr_public_coeffs_config_t;


typedef struct awb_fr_public_nf_config {
    //TODO - this is calculate by log2 of the sum ofcoeffs, should it be exposed to user?
    unsigned char bayer_nf;

} awb_fr_public_nf_config_t;

/**
* constraint: this grid has to be totally internal to the processed frame plane
*             with margins of 10 per horizontal direction and 2 per vertical direction
*             therefore, we'll set its default value to represent the smallest size;
*             this will restrain the processed resolution the less
*/
struct awb_fr_public_grid_config{
    unsigned char grid_width;	/**< default value 16, Note: according to HAS - 32 */
    unsigned char grid_height;	/**< default value 16, Note: according to HAS - 24 */
    unsigned char block_width;  /**< default value 3, Log2 the width of each cell (8,16,32,64,128) */
    unsigned char block_height;	/**< default value 3, Log2 the width of each cell (8,16,32,64,128) */
    unsigned short x_start;	/**< default value 10, X top left corner of the grid x_start is even */
    unsigned short y_start;	/**< default value  2, Y top left corner of the grid y_start is even */
    unsigned char af_bayer_en;	/**< default value  1, awb_fr ff enable/disable meta data generation */
};

struct awb_fr_public_config {
    awb_fr_public_coeffs_config_t bayer_coeffs;
    awb_fr_public_nf_config_t     nf;
    struct awb_fr_public_grid_config   grid;
};

struct ia_css_2500_awb_fr_kernel_config {
    struct awb_fr_public_config awb_fr;
};

// awb_fr public meta-data
typedef struct awb_fr_public_bayer_item{
    unsigned short gr_avg; /**< Average level of GreenR color */
    unsigned short r_avg;  /**< Average level of Red color */
    unsigned short b_avg;  /**< Average level of Blue color */
    unsigned short gb_avg; /**< Average level of GreenB color */

} awb_fr_public_bayer_item_t;

/** Based on max grid width + spare for bubbles */
#define AWB_FR_PUBLIC_NUM_OF_ITEMS_IN_SET  (AWB_FR_MAX_GRID_WIDTH + 20)
#define AWB_FR_PUBLIC_NUM_OF_SETS_IN_BUFFER AWB_FR_MAX_GRID_HEIGHT

#define AWB_FR_BUFF_RATIO 2 /**< AWB_FR stats buffer ratio */

typedef struct awb_fr_public_raw_buffer{
	awb_fr_public_bayer_item_t bayer_table[AWB_FR_PUBLIC_NUM_OF_ITEMS_IN_SET *
					       AWB_FR_PUBLIC_NUM_OF_SETS_IN_BUFFER * AWB_FR_BUFF_RATIO]; /**< awb_fr statistics table */
} awb_fr_public_raw_buffer_t;

/** @brief Print AWB_FR public configuration
 *
 * @param	cfg	The pointer to configuration data
 * @return	None
 *
 * Print AWB_FR public configuration.
 */
void ia_css_awb_fr_public_cfg_dump(
	const struct ia_css_2500_awb_fr_kernel_config *cfg);

/** @brief Compare two AWB_FR public configurations
 *
 * @param	cfg1	The pointer to first configuration data
 * @param	cfg2	The pointer to second configuration data
 * @param	cfg_dump	Configurations are printed in case of
 *   			   mismatch
 * @return	true - match, false - not match
 *
 * Compare two AWB_FR public configurations
 */
bool ia_css_awb_fr_public_cfg_compare(
	const struct ia_css_2500_awb_fr_kernel_config *cfg1,
	const struct ia_css_2500_awb_fr_kernel_config *cfg2,
	bool cfg_dump);

#endif /* _AWB_FR_PUBLIC_H_ */
