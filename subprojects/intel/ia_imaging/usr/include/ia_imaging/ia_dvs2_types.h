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
/** @file ia_dvs2_types.h
 * IA_DVS2 data types. This provides data types to access to the DVS2 Host Library.
 */
#ifndef _IA_DVS2_TYPES_H_
#define _IA_DVS2_TYPES_H_
#include <stdint.h>
#include "ia_types.h"

/** DVS2 Algorithm.
 * These settings specify the nembers of correction axes.
 * ia_dvs2_algorihm_0_axis is special mode. In this case, library does not compensate the
 * motion, works for digital zoom / scaling.
 */
typedef enum {
        ia_dvs2_algorihm_0_axis = 0,                         /**< 0 axis, means digital zoom/scaling mode */
        ia_dvs2_algorihm_2_axis = 2,                         /**< 2 axis */
        ia_dvs2_algorihm_4_axis = 4,                         /**< 4 axis */
        ia_dvs2_algorihm_6_axis = 6,                         /**< 6 axis */
        ia_dvs2_algorihm_max_axis = ia_dvs2_algorihm_6_axis, /**< maximum axis */
} ia_dvs2_algorithm_version;

/** DVS2 resolution configuration.
 * These parameter are DVS2 resolution configuration.
 */
typedef struct {
        int width;            /**< width [pixel] */
        int height;           /**< height [pixel] */
} ia_dvs2_resolution;

/** DVS2 BQ resolution.
 * These parameter are BQ resolution configuration.
 */
typedef struct {
        int width_bq;         /**< width [BQ] */
        int height_bq;        /**< height [BQ] */
} ia_dvs2_bq_resolution;

/** GDC Scan Mode
 * These settings specify the gdc scan mode.
 */
typedef enum {
        ia_dvs2_gdc_scan_mode_stb = 0,  /**< STB (slide to the bottom) */
        ia_dvs2_gdc_scan_mode_str,      /**< STR (slide to the right) */
} ia_dvs2_gdc_scan_mode;

/** GDC Interpolation Method
 * These settings specify the gdc interpolation method.
 */
typedef enum {
        ia_dvs2_gdc_interpolation_nnd = 0,  /**< NND (nearest neighbor) */
        ia_dvs2_gdc_interpolation_bli,      /**< BLI (bilinear) */
        ia_dvs2_gdc_interpolation_bci,      /**< BCI (bicubic) */
        ia_dvs2_gdc_interpolation_lut,      /**< LUT (look up table) */
} ia_dvs2_gdc_interpolation;

/** GDC Performance Point
 * These settings specify the gdc performance point.
 */
typedef enum {
        ia_dvs2_gdc_performance_point_1x1 = 0,  /**< 1x1 */
        ia_dvs2_gdc_performance_point_2x1,      /**< 2x1 */
        ia_dvs2_gdc_performance_point_1x2,      /**< 1x2 */
        ia_dvs2_gdc_performance_point_2x2,      /**< 2x2 */
} ia_dvs2_gdc_performance_point;

/** GDC hardware configuration
 * These parametes are the gdc hardware block configuration.
 * dvs2 library use these parameters just check the gdc constraints,
 * do NOT use these parameters for any controls nor calculations.
 */
typedef struct {
        ia_dvs2_gdc_scan_mode scan_mode;
        ia_dvs2_gdc_interpolation interpolation;
        ia_dvs2_gdc_performance_point performance_point;
} ia_dvs2_gdc_hw_configuration;

/** GDC distortion coefficients.
 * This structure contains GDC distortion coefficients.
 */
typedef struct {
        float gdc_k1;                /**< Distortion Coefficient K1 */
        float gdc_k2;                /**< Distortion Coefficient K2 */
        float gdc_k3;                /**< Distortion Coefficient K3 */
        float gdc_p1;                /**< Distortion Coefficient P1 */
        float gdc_p2;                /**< Distortion Coefficient P2 */
} ia_dvs2_distortion_coefs;

/** DVS configuration.
 * This structure contains DVS configuration.
 */
typedef struct {
        ia_dvs2_algorithm_version num_axis;             /**< algorithm */
        float nonblanking_ratio;                        /**< effective vertical scan ratio, used for rolling correction (Non-blanking ration of frame interval) */
        int grid_size;                                  /**< isp process grid size [BQ] */
        ia_dvs2_bq_resolution source_bq;                /**< GDC source image size [BQ] */
        ia_dvs2_bq_resolution output_bq;                /**< GDC output image size [BQ] */
        ia_dvs2_bq_resolution envelope_bq;              /**< GDC effective envelope size [BQ] */
        ia_dvs2_bq_resolution ispfilter_bq;             /**< isp pipe filter size [BQ] */
        int gdc_shift_x;                                /**< shift value of morphing table depend on ISP pipe. [chroma pixel] */
        int gdc_shift_y;                                /**< shift value of morphing table depend on ISP pipe. [chroma pixel] */
        unsigned int oxdim_y;                           /**< output block width  for Y plane [pixel] */
        unsigned int oydim_y;                           /**< output block height for Y plane [pixel] */
        unsigned int oxdim_uv;                          /**< output block width  for U/V plane [chroma pixel] */
        unsigned int oydim_uv;                          /**< output block height for U/V plane [chroma pixel] */
        ia_dvs2_gdc_hw_configuration hw_config;         /**< GDC h/w configuration */
} ia_dvs2_configuration;

/** DVS2 text log setup.
 * This structure contains the text log infomation.
 */
typedef struct {
        const char *path;       /**< path of log file */
        int enable;             /**< enable/disable of the log function */
} ia_dvs2_log_setup;

/** DVS2 binary dump data type.
 * This enum contains the binary dump record data type.
 */
typedef enum {
        eBDSupportConfig,
        eBDGdcConfig,
        eBDCharacteristics,
        eBDBasicConfig,
        eBDDigitalZoomRatio,
        eBDDvisParam,
        eBDGdcParam,
        eBDMatMotion,
        eBDMatRoll,
        eBDTimeStamp,
        eBDFrameCount,
        eBDBorderFlag,
        eBDMotionVector,
        eBDVProd,
        eBDHProd,
        eBDLocalMotionDyn,
        eBDLocalMotionDxn,
        eBDLocalMotionWy,
        eBDLocalMotionWx,
        eBDVProdRawEvenReal,
        eBDVProdRawEvenImag,
        eBDVProdRawOddReal,
        eBDVProdRawOddImag,
        eBDHProdRawEvenReal,
        eBDHProdRawEvenImag,
        eBDHProdRawOddReal,
        eBDHProdRawOddImag,
        eBDCompMotionV,
        eBDCompMotionH,
        eBDMorphingTableYH,
        eBDMorphingTableYV,
        eBDMorphingTableUVH,
        eBDMorphingTableUVV,
        eBDsetNonBlankingRatio,
        eBDsetMinLocalMotion,
        eBDsetCutOffFrequency,
        eBDsetDistortionCoeff,
        eBDsetWaveLength,
        eBDVelocity,
        eBDPrevMotion,
        eBDPrevDisplace,
        eBDDisplaceLimit,
        /*---------*/
        eBDNumsItem
        /* Do NOT add any items from here. */
} ia_dvs2_binary_dump_item;

/** DVS2 binary dump parameter.
 * This structure contains parameter for binary dump.
 */
typedef struct {
        int frames;
        bool endless;
        int binaryDumpFailed;
} ia_dvs2_binary_dump_params;
#endif // _IA_DVS2_TYPES_H_
