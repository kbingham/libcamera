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
/*!
 * \file ia_dvs_types.h
 * \brief Data types and enumerations for Intel DVS library.
 */
#ifndef _IA_DVS_TYPES_H_
#define _IA_DVS_TYPES_H_
#include <stdint.h>
#include "ia_types.h"

typedef struct t_dvs_facade ia_dvs_state;

/** DVS Algorithm.
 * These settings specify the members of correction axes.
 * ia_dvs_algorihm_0_axis is a special mode. In this case, library does not compensate the
 * motion, works for digital zoom and distortion correction.
 */
typedef enum
{
    ia_dvs_algorithm_0_axis     = 0,                        /**< 0 axis, means video stabilization is turned off */
    ia_dvs_algorithm_2_axis     = 2,                        /**< 2 axis - x and y translations */
    ia_dvs_algorithm_3_axis     = 3,                        /**< 3 axis - x and y translations and z-scale*/
    ia_dvs_algorithm_4_axis     = 4,                        /**< 4 axis */
    ia_dvs_algorithm_5_axis     = 5,                        /**< 5 axis - x and y translations and all rotations*/
    ia_dvs_algorithm_6_axis     = 6,                        /**< 6 axis - x and y translations, z-scale and all rotations*/
    ia_dvs_algorithm_max_axis   = ia_dvs_algorithm_6_axis,  /**< maximum axis */
    ia_dvs_algorithm_motion_sensor = 10                     /**< motion sensor based stabilization */
} ia_dvs_algorithm_version;

/** DVS2 BQ resolution.
 * These parameter are BQ resolution configuration.
 */
typedef struct
{
    int width_bq;                           /**< width [BQ] */
    int height_bq;                          /**< height [BQ] */
} ia_dvs_bq_resolution;

/** GDC Scan Mode
 * These settings specify the gdc scan mode.
 */
typedef enum
{
    ia_dvs_gdc_scan_mode_stb    = 0,        /**< STB (slide to the bottom) */
    ia_dvs_gdc_scan_mode_str,               /**< STR (slide to the right) */
} ia_dvs_gdc_scan_mode;

/** GDC Interpolation Method
 * These settings specify the gdc interpolation method.
 */
typedef enum
{
    ia_dvs_gdc_interpolation_nnd= 0,        /**< NND (nearest neighbor) */
    ia_dvs_gdc_interpolation_bli,           /**< BLI (bilinear) */
    ia_dvs_gdc_interpolation_bci,           /**< BCI (bicubic) */
    ia_dvs_gdc_interpolation_lut,           /**< LUT (look up table) */
} ia_dvs_gdc_interpolation;

/** GDC Performance Point
 * These settings specify the gdc performance point.
 */
typedef enum
{
    ia_dvs_gdc_performance_point_1x1= 0,    /**< 1x1 */
    ia_dvs_gdc_performance_point_2x1,       /**< 2x1 */
    ia_dvs_gdc_performance_point_1x2,       /**< 1x2 */
    ia_dvs_gdc_performance_point_2x2,       /**< 2x2 */
} ia_dvs_gdc_performance_point;

/** GDC hardware configuration
 * These parametes are the gdc hardware block configuration.
 * dvs library use these parameters just check the gdc constraints,
 * do NOT use these parameters for any controls nor calculations.
 */
typedef struct
{
    ia_dvs_gdc_scan_mode scan_mode;
    ia_dvs_gdc_interpolation interpolation;
    ia_dvs_gdc_performance_point performance_point;
} ia_dvs_gdc_hw_configuration;

/** GDC buffer configuration
 *  These parameters indicates the limits of the GDC ISP buffer.
 *  DVS needs to limit morphing table coordinates to fit inside GDC
 *  ISP buffer.
 */
typedef struct
{
    unsigned int x_offset;  /* X offset [BQ] for the first pixel of image data */
    unsigned int y_offset;  /* Y offset [BQ] for the first pixel of image data*/
    unsigned int width;     /* Total width [BQ] for the buffer */
    unsigned int height;    /* Total height [BQ] for the buffer */
} ia_dvs_gdc_buffer_config;

/** Total cropping parameters
 *  These parameters contain sensor and ISP cropping information without any scaling or binning.
 *  In case of scaling before cropping, cropping params needs to be scaled back to the original resolution.
 *  This information is used to crop lens distortion grids accordingly.
 */
typedef struct
{
    unsigned int horizontal_crop_offset;    /* Read out offset horizontal [BQ] */
    unsigned int vertical_crop_offset;      /* Read out offset vertical [BQ] */
    unsigned int cropped_width;             /* Width of cropped area without any scaling [BQ] */
    unsigned int cropped_height;            /* Height of cropped area without any scaling [BQ] */
} ia_dvs_crop_params;

/** DVS configuration.
 * This structure contains DVS configuration.
 */
typedef struct
{
    ia_dvs_algorithm_version num_axis;          /**< Algorithm mode */
    float nonblanking_ratio;                    /**< Effective vertical scan ratio, used for rolling correction (Non-blanking ration of frame interval) */
    ia_dvs_bq_resolution source_bq;             /**< Input image size [BQ] for GDC block */
    ia_dvs_bq_resolution output_bq;             /**< Output image size [BQ] from GDC block */
    ia_dvs_bq_resolution envelope_bq;           /**< GDC effective envelope size [BQ] */
    ia_dvs_bq_resolution ispfilter_bq;          /**< Padding of the image which is corrupted and should not be visible in the output image [BQ] */
    int gdc_shift_x;                            /**< Shift value of morphing table depend on ISP pipe. [chroma pixel] */
    int gdc_shift_y;                            /**< Shift value of morphing table depend on ISP pipe. [chroma pixel] */
    unsigned int oxdim_y;                       /**< Output block width  for Y plane [pixel] */
    unsigned int oydim_y;                       /**< Output block height for Y plane [pixel] */
    unsigned int oxdim_uv;                      /**< Output block width  for U/V plane [chroma pixel] */
    unsigned int oydim_uv;                      /**< Output block height for U/V plane [chroma pixel] */
    ia_dvs_gdc_hw_configuration hw_config;      /**< GDC h/w configuration. DVS does not set these values anywhere so it's only
                                                     used when validating outgoing morphing table. */
    bool use_lens_distortion_correction;        /**< False disables LDC, true enables */
    int frame_rate;                             /**< Frame rate */
    ia_dvs_gdc_buffer_config gdc_buffer_config; /**< Configuration of the GDC buffer is used inside DVS to prevent morphing table to point
                                                     invalid memory locations in GDC.

                                                     This configuration should come from FW
                                                     to inform how large buffer is allocated for GDC processing.
                                                     Morphing table coordinates generated by DVS must fit inside
                                                     GDC buffer limits. GDC buffer is allocated by FW and
                                                     it needs to allocate extra padding for each side of the image data.
                                                     This allows DVS to generate morphing table which points are outside of the image area
                                                     but still inside GDC buffer (padding area). This might be the case e.g. if LDC grid is
                                                     barrel shaped. */
    ia_dvs_crop_params crop_params;             /**< Sensor and ISP cropping parameteres in native resolution (without any scaling or binning).
                                                     If scaling is performed before cropping, cropping offsets need to be calculated in native
                                                     resolution. Also if cropping is done in multiple places, everything needs to be combined
                                                     together.
                                                     Lens distortion correction is calculated from the full sensor resolution and
                                                     DVS needs to know how distortion grids need to be cropped so that they will match with
                                                     the GDC input image.*/
    bool validate_morph_table;                  /**< False disables morph table validation, true enables. Morph table needs to be validated to
                                                     protect FW. Starting from IPU4 validation is performed in PAL. */
} ia_dvs_configuration;

/** Distortion grid configuration.
 * Structure defines lens distortion grid.
 */
typedef struct
{
    int16_t GDC_col_start;      /*!< Table X offset in pixels from left corner of the sensor maximum visible area.
                                     e.g. If GDC_col_start=GDC_block_width*(-1)
                                     then GDC table offset is is one block left compared to the maximum visible sensor area. */
    int16_t GDC_row_start;      /*!< Table Y offset in pixels from upper corner of the sensor maximum visible area.
                                     e.g. If GDC_row_start=GDC_block_height*(-1)
                                     then GDC table offset is is one block up compared to the maximum visible sensor area.  */
    int16_t GDC_grid_width;     /*!< Indicates number of grid vertices on the horizontal axis. */
    int16_t GDC_grid_height;    /*!< Indicates number of grid vertices on the vertical axis. */
    int16_t GDC_block_width;    /*!< Width of the original grid cell (without correction). */
    int16_t GDC_block_height;   /*!< Height of the original grid cell (without correction). */
    float *x_deltas;            /*!< Table of x-axis deltas of the grid points. The delta at each point represents the distortion
                                     that was done. Contains [GDC_grid_height  x GDC_grid_width] values. */
    float *y_deltas;            /*!< Table of y-axis deltas of the grid points. The delta at each point represents the distortion
                                     that was done. Contains [GDC_grid_height  x GDC_grid_width] values. */
} ia_dvs_distortion_config;

/** DVS Motion vector structure.
 * This structure contains definition for one local motion vector.
 */
typedef struct
{
    float x_start;          /* Normalized X start position */
    float y_start;          /* Normalized Y start position */
    float x_end;            /* Normalized X end position */
    float y_end;            /* Normalized y end position */
    float weight;           /* Weight of the motion vector [0.0, 1.0]. Describes accuracy of the motion vector */
} ia_dvs_motion_vector;

/** DVS Motion vectors.
 * This structure contains DVS statistics.
 */
typedef struct
{
    unsigned int vector_count;              /* Number of motion vectors */
    ia_dvs_motion_vector *motion_vectors;   /* Table of local motion vectors. Contains [vector_count] values. */
} ia_dvs_statistics;

/** DVS morphing table structure.
 * This structure contains morphing table which includes
 * lens distortion correction, digital zoom, rolling shutter correction and video stabilization.
 */
typedef struct
{
    uint32_t width_y;
    uint32_t height_y;
    uint32_t width_uv;
    uint32_t height_uv;
    uint32_t *xcoords_y;
    uint32_t *ycoords_y;
    uint32_t *xcoords_uv;
    uint32_t *ycoords_uv;
    bool morph_table_changed;
} ia_dvs_morph_table;

/** Digital zoom mode
 * These settings specify digital zoom mode.
 */
typedef enum {
        ia_dvs_zoom_mode_center = 0,
        ia_dvs_zoom_mode_region,
        ia_dvs_zoom_mode_coordinate
} ia_dvs_zoom_mode;

/** DVS global translation parameters.
 * This structure contains the frame-to-frame translations along x and y axes
 * and also a coefficient for scene reliability in the range [0, 1] with 1 being
 * totally reliable estimate and 0 being totally unreliable
 */
typedef struct
{
    uint32_t plane_translation_x;
    uint32_t plane_translation_y;
    float reliability_coefficient;
} ia_dvs_global_translation;

#endif /* _IA_DVS_TYPES_H_ */
