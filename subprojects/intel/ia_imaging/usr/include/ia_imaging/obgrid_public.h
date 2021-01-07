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

#ifndef _OBGRID_PUBLIC_H_
#define _OBGRID_PUBLIC_H_

/** @file
* CSS-API header file for 2500/Skycam black level correction (optical black, OB) kernel.
*
* This file contains Configuration parameters for grid-based black level correction
*/

#define IA_CSS_OBGRID_TILE_SIZE         16
/** width; max grid size is derived from the max frame size as seen from obgrid perspective (4618x3464) */
#define IA_CSS_OBGRID_MAX_GRID_WIDTH    146
/** height; max grid size is derived from the max frame size as seen from obgrid perspective (4618x3464) */
#define IA_CSS_OBGRID_MAX_GRID_HEIGHT   110
/** size; max grid size is derived from the max frame size as seen from obgrid perspective (4618x3464) */
#define IA_CSS_OBGRID_MAX_GRID_SIZE     (IA_CSS_OBGRID_MAX_GRID_WIDTH * IA_CSS_OBGRID_MAX_GRID_HEIGHT)

/**
 * Description of the struct members:
 *
 * - tile_size: Width and height of a single grid tile.
 *      Valid value is IA_CSS_OBGRID_TILE_SIZE. Any other value is not supported.
 *      This field is provided for future extensions only. The tile size is
 *      given as a number of pixels of the same bayer color. For example, a
 *      16x16 tile size covers a 32x32 area in the image.
 *
 * - x_offset: X value of the top-left grid point relative to ROI.
 * - y_offset: Y value of the top-left grid point relative to ROI.
 *      Valid value is 0. Any other value is not supported.
 *      These fields are provided for future extensions only.
 *
 * - table_width: Width of the grid.
 *      Valid values are between 1 and IA_CSS_OBGRID_MAX_GRID_WIDTH.
 * - table_height: Heigth of the grid.
 *      Valid values are between 1 and IA_CSS_OBGRID_MAX_GRID_HEIGHT.
 *      To cover the entire image, the width and height must be at least
 *      image_size/tile_size + 1. If the grid is not wide or high enough,
 *      then it will be extended by repeating the right or bottom elements.
 *
 * - table_GR: Grid table values for color Gr.
 * - table_R:  Grid table values for color R.
 * - table_B:  Grid table values for color B.
 * - table_GB: Grid table values for color Gb.
 *      Valid values are between 0 and (2^N)-1, where N is the ISP bit width
 *      minus 1. On skycam, N is 11 bits. Each table must contain at least
 *      table_width*table_height valid elements, in row-major order.
 */
struct ia_css_2500_obgrid_kernel_config
{
    int   x_offset;                               /**< X offset of the grid origin */
    int   y_offset;                               /**< Y offset of the grid origin */
    int   tile_size;                              /**< Grid tile size */
    int   table_width;                            /**< Grid width */
    int   table_height;                           /**< Grid height */
    short table_GR[IA_CSS_OBGRID_MAX_GRID_SIZE];  /**< Grid values for Gr */
    short table_R [IA_CSS_OBGRID_MAX_GRID_SIZE];  /**< Grid values for R */
    short table_B [IA_CSS_OBGRID_MAX_GRID_SIZE];  /**< Grid values for B */
    short table_GB[IA_CSS_OBGRID_MAX_GRID_SIZE];  /**< Grid values for Gb */
};

#endif /* _OBGRID_PUBLIC_H_ */
