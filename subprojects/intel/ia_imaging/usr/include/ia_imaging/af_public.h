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

#ifndef _AF_PUBLIC_H_
#define _AF_PUBLIC_H_

/** @file
* CSS-API header file for 2500/Skycam Auto Focus (AF) kernel/accelerator.
*/
//#include <type_support.h>

/* All structs in this file will be visible to the CSS APU User.
 * The User will be able to set each one of the config params separately by
 * specifying a different config type (see sh_config_type in sh_css.h).
 * If one of those config structs is not set explicitly, a default value will
 * be use instead.
 */

#define AF_MIN_GRID_WIDTH   16
#define AF_MIN_GRID_HEIGHT  16
#define AF_MAX_GRID_WIDTH   32
#define AF_MAX_GRID_HEIGHT  24
#define AF_MIN_BLOCK_WIDTH  3
#define AF_MIN_BLOCK_HEIGHT 3
#define AF_MAX_BLOCK_WIDTH  6 /**< HSD1403986067 bug in c0, restrict max block width to 6 instead of 7 */
#define AF_MAX_BLOCK_HEIGHT 6 /**< HSD1403986067 bug in c0, restrict max block height to 6 instead of 7 */

typedef struct af_public_coeffs_config{
	unsigned char A1;	/**< default value 0 */
	unsigned char A2;	/**< default value 0 */
	unsigned char A3;	/**< default value 0 */
	unsigned char A4;	/**< default value 0 */
	unsigned char A5;	/**< default value 0 */
	unsigned char A6;	/**< default value 0 */
	unsigned char A7;	/**< default value 0 */
	unsigned char A8;	/**< default value 0 */
	unsigned char A9;	/**< default value 0 */
	unsigned char A10;	/**< default value 0 */
	unsigned char A11;	/**< default value 0 */
	unsigned char A12;	/**< default value 128 */
	unsigned int  sign_vec;
} af_public_coeffs_config_t;

/** constraint:
* y_gen_rate_gr + y_gen_rate_r + y_gen_rate_b + y_gen_rate_gb = 32
*/
typedef struct af_public_y_calc_config{
	unsigned char y_gen_rate_gr; /**< default value 8 */
	unsigned char y_gen_rate_r;  /**< default value 8 */
	unsigned char y_gen_rate_b;  /**< default value 8 */
	unsigned char y_gen_rate_gb; /**< default value 8 */
} af_public_y_calc_config_t;

typedef struct af_public_nf_config{
	/* TODO - do these numbers need to be calculated from the other config params? */
	  unsigned char y1_nf; /**< default value 7 */
	  unsigned char y2_nf; /**< default value 7 */
} af_public_nf_config_t;

/**
* constraint: this grid has to be totally internal to the processed frame plane
*             with margins of 10 per horizontal direction and 2 per vertical direction
*             therefore, we'll set its default vaule to represent the smallest size;
*             this will restrain the processed resolution the less
*/
struct af_public_grid_config{
	unsigned char grid_width;	/**< default value 16 */
	unsigned char grid_height;	/**< default value 16 */
	unsigned char block_width;	/**< default value 3, Log2 the width of each cell (8,16,32,64,128) */
	unsigned char block_height;	/**< default value 3, Log2 the height of each cell  (8,16,32,64,128) */
	unsigned short x_start; 	/**< default value 10, X top left corner of the grid x_start is even */
	unsigned short y_start;	 	/**< default value  2, Y top left corner of the grid y_start is even */
	unsigned char af_y_en;		/**< default value 1, af ff enable/disable meta data generation */
} ;

struct af_public_config {
	af_public_coeffs_config_t		y1_coeffs;
	af_public_coeffs_config_t 		y2_coeffs;
	af_public_y_calc_config_t 		y_calc;
	af_public_nf_config_t     		nf;
	struct af_public_grid_config	grid;
};

struct ia_css_2500_af_kernel_config {
    struct af_public_config af;
};

#define AF_PUBLIC_NUM_OF_SETS_IN_BUFFER AF_MAX_GRID_HEIGHT
/** Based on max grid width + Spare for bubbles */
#define AF_PUBLIC_NUM_OF_ITEMS_IN_SET   (AF_MAX_GRID_WIDTH + 20)

/* af public meta-data */
typedef struct af_public_y_item{
    unsigned short y1_avg;
    unsigned short y2_avg;
} af_public_y_item_t;

#define AF_BUFF_RATIO 2 /**< AF stats buffer ratio */

typedef struct af_public_raw_buffer{
	af_public_y_item_t y_table[AF_PUBLIC_NUM_OF_SETS_IN_BUFFER * AF_PUBLIC_NUM_OF_ITEMS_IN_SET *
				   AF_BUFF_RATIO]; /**< af statistics table */
} af_public_raw_buffer_t;

/** @brief Print AF public configuration
 *
 * @param	cfg	The pointer to configuration data
 * @return	None
 *
 * Print AF public configuration.
 */
void ia_css_af_public_cfg_dump(const struct ia_css_2500_af_kernel_config *cfg);

/** @brief Compare two AF public configurations
 *
 * @param	cfg1	The pointer to first configuration data
 * @param	cfg2	The pointer to second configuration data
 * @param	cfg_dump	Configurations are printed in case of
 *   			   mismatch
 * @return	true - match, false - not match
 *
 * Compare two AF public configurations
 */
bool ia_css_af_public_cfg_compare(
	const struct ia_css_2500_af_kernel_config *cfg1,
	const struct ia_css_2500_af_kernel_config *cfg2,
	bool cfg_dump);
#endif /* _AF_PUBLIC_H_ */
