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
 * \file ia_isp_types.h
 * \brief Common ISP enumerations and structures.
*/


#ifndef IA_ISP_TYPES_H_
#define IA_ISP_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  \brief Complexity level for ISP features.
 */
typedef enum
{
    ia_isp_feature_level_off,   /* Feature is turned off */
    ia_isp_feature_level_low,   /* Minimum set of algorithms are used */
    ia_isp_feature_level_high   /* Maximum set of algorithms are used */
} ia_isp_feature_level;

/*!
 * \brief Custom control parameters for tuning interpolation in GAIC
 *
 * GAIC implements generic interpolations of tuning parameters as factor of certain run-time changing parameter.
 * This structure defines custom run-time changeable control parameters, which can be used in tuning interpolations.
 */
typedef struct
{
    int count;                        /*!< Length of parameters array. */
    float *parameters;                /*!< Parameters used in calculation (interpolation) of tuning values.
                                           The first element of the array corresponds custom tuning 0,
                                           second corresponds with custom tuning 1 etc.*/
} ia_isp_custom_controls;

/*!
 *  \brief Definitions for the color effects.
 */
typedef enum
{
    ia_isp_effect_none     =               0,
    ia_isp_effect_sky_blue =         (1 << 0),
    ia_isp_effect_grass_green =      (1 << 1),
    ia_isp_effect_skin_whiten_low =  (1 << 2),
    ia_isp_effect_skin_whiten =      (1 << 3),
    ia_isp_effect_skin_whiten_high = (1 << 4),
    ia_isp_effect_sepia =            (1 << 5),
    ia_isp_effect_black_and_white =  (1 << 6),
    ia_isp_effect_negative =         (1 << 7),
    ia_isp_effect_vivid =            (1 << 8),
    ia_isp_effect_invert_gamma =     (1 << 9),
    ia_isp_effect_grayscale =        (1 << 10),
    ia_isp_effect_aqua =             (1 << 11)
} ia_isp_effect;

/*!
 *  \brief Settings for feature level and strength.
 */
typedef struct
{
    ia_isp_feature_level feature_level;  /* Feature level */
    char strength;                       /* Setting for the strength [-128,127]. */
} ia_isp_feature_setting;

/* Thresholds to extract data of given range from statistics. Used to split combined WDR statistics into several low dynamic range statistics. */
typedef struct
{
    float low;
    float high;
    float scale;
} ia_isp_stat_split_thresh;

#ifdef __cplusplus
}
#endif

#endif /* IA_ISP_TYPES_H_ */
