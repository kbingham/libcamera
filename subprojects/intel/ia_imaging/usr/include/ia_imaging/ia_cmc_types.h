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
 * \file ia_cmc_types.h
 * \brief Definitions of Camera Module Characterization (CMC) records.
*/

#ifndef IA_CMC_TYPES_H_
#define IA_CMC_TYPES_H_

#include "ia_mkn_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* All CMC records are inside AIQB record in CPFF file. */
#define AIQB_TAG IA_MKN_CHTOUL('A','I','Q','B')  /*!< AIQ configuration block tag. */

/*
 * Color Channels (1-4) refer to Raw Bayer quad in the following order.
 * -------------
 * | CC1 | CC2 |
 * -------------
 * | CC3 | CC4 |
 * -------------
 *
 * Structures defined in this header file, which are stored into memory/file must start from 64 bit boundary (for 64 bit systems).
 * Keep in mind also structures' internal padding when defining new structures.
 */
#define CMC_NUM_CHANNELS 4

typedef struct
{
    uint16_t cc1;
    uint16_t cc2;
    uint16_t cc3;
    uint16_t cc4;
} cmc_color_channels;

/*!
 * \brief CIE x and Y coordinates.
 */
typedef struct
{
    uint16_t x;
    uint16_t y;
} cie_coords_t;

/*!
 * \brief R/G and B/G ratios.
 */
typedef struct
{
    uint16_t r_per_g;
    uint16_t b_per_g;
} chromaticity_t;

/*!
 * \brief Physical pixel coordinates.
 */
typedef struct
{
    uint16_t x;
    uint16_t y;
} cmc_coords;

/*!
 * \brief Chromaticity coordinates in CIE 1931 space.
 */
typedef struct
{
    float x;
    float y;
} cmc_cie_coords;

/*!
 * \brief Chromaticity coordinates in R/G & B/G space.
 */
typedef struct
{
    float r_per_g;
    float b_per_g;
} cmc_chromaticity;

/*!
* \brief 3x3 color matrix.
*/
typedef struct
{
    float matrix[3][3];                 /*!< 3x3 accurate CCM, each consequtive elements sum to 1. */
} cmc_color_matrix;

/*!
 * \brief Raw image bayer order.
 */
typedef enum
{
    cmc_bayer_order_grbg = 0,  /*!< First row contains pixels Gr, R. Second row contains pixels B, Gb. */
    cmc_bayer_order_rggb = 1,  /*!< First row contains pixels R, Gr. Second row contains pixels Gb, B. */
    cmc_bayer_order_bggr = 2,  /*!< First row contains pixels B, Gb. Second row contains pixels Gr, R. */
    cmc_bayer_order_gbrg = 3,  /*!< First row contains pixels Gb, B. Second row contains pixels R, Gr. */
} cmc_bayer_order;

/*!
 * \brief CMC names used in the record headers.
 */
typedef enum
{
    cmc_name_id_reserved = 0,                            /*!< 00 */
    cmc_name_id_comment,                                 /*!< 01 */
    cmc_name_id_general_data,                            /*!< 02 */
    cmc_name_id_black_level,                             /*!< 03 */
    cmc_name_id_black_level_spatial,                     /*!< 04 */
    cmc_name_id_saturation_level,                        /*!< 05 */
    cmc_name_id_dynamic_range_and_linearity,             /*!< 06 */
    cmc_name_id_module_sensitivity,                      /*!< 07 */
    cmc_name_id_defect_pixels,                           /*!< 08 */
    cmc_name_id_noise,                                   /*!< 09 */
    cmc_name_id_lens_shading_correction,                 /*!< 10 */
    cmc_name_id_lens_shading_correction_ratio,           /*!< 11 */
    cmc_name_id_geometric_distortion_correction,         /*!< 12 */
    cmc_name_id_optics_and_mechanics,                    /*!< 13 */
    cmc_name_id_module_spectral_response,                /*!< 14 */
    cmc_name_id_chromaticity_response,                   /*!< 15 */
    cmc_name_id_flash_chromaticity,                      /*!< 16 */
    cmc_name_id_nvm_info,                                /*!< 17 */
    cmc_name_id_color_matrices,                          /*!< 18 */
    cmc_name_id_analog_gain_conversion,                  /*!< 19 */
    cmc_name_id_digital_gain,                            /*!< 20 */
    cmc_name_id_sensor_metadata,                         /*!< 21 */
    cmc_name_id_geometric_distortion_correction2,        /*!< 22 */
    cmc_name_id_exposure_range,                          /*!< 23 */
    cmc_name_id_multi_led_flash_chromaticity,            /*!< 24 */
    cmc_name_id_advanced_color_matrices,                 /*!< 25 */
    cmc_name_id_hdr,                                     /*!< 26 */
    cmc_name_id_infrared_correction,                     /*!< 27 */
    cmc_name_id_lens_shading_correction_4x4,             /*!< 28 */
    cmc_name_id_lateral_chromatic_aberration_correction  /*!< 29 */
} cmc_name_id;

/*!
 * \brief Camera features flags (see cmc_optomechanics_t.camera_features).
 * Bit '1' means that the feature is presented.
 */
typedef enum {
    cmc_camera_feature_lens_position_sensor        = 1,            /*!< Physical Lens Position sensor */
    cmc_camera_feature_voice_coil_actuator         = (1 << 1),     /*!< 'Voice Coil' type of lens actuator */
    cmc_camera_feature_hybrid_voice_coil_actuator  = (1 << 2),     /*!< 'Hybrid Voice Coil' type of lens actuator */
    cmc_camera_feature_piezo_actuator              = (1 << 3),     /*!< 'Piezo' type of lens actuator */
    cmc_camera_feature_mems_actuator               = (1 << 4),     /*!< 'MEMS' type of lens actuator */
    cmc_camera_feature_nd_filter                   = (1 << 5),     /*!< Neutral Density filter */
    cmc_camera_feature_mechanical_shutter          = (1 << 6),     /*!< Mechanical Shutter */
    cmc_camera_feature_variable_apertures          = (1 << 7),     /*!< Variable Apertures */
    cmc_camera_feature_optical_zoom                = (1 << 8)      /*!< Optical Zoom */
} cmc_camera_feature;

/*!
 * \brief Camera module orientation
 */
typedef enum {
    cmc_camera_orientation_down = 0,            /*!< Camera module is pointing down */
    cmc_camera_orientation_horizontally,        /*!< Camera module is pointing horizontally */
    cmc_camera_orientation_up                   /*!< Camera module is pointing up */
} cmc_camera_orientation;

/*!
 * \brief LSC level enumeration.
 */
typedef enum {
    cmc_lsc_force_first_grid = 0,           /*!< Use always first LSC grid */
    cmc_lsc_without_nvm,                    /*!< Use LSC adaptation without NVM */
    cmc_lsc_with_nvm                        /*!< Use LSC adaptation with NVM */
} cmc_lsc_level;

/*!
 * \brief Shading Adaptor Level enumeration.
 */
typedef enum {
    cmc_sa_disable = 0,                     /*!< Do not apply shading tables (bypass LSC) */
    cmc_sa_cct_based,                       /*!< Use CCT based LSC selection */
    cmc_sa_adaptive,                        /*!< Use adaptive LSC selection */
    cmc_sa_self_adjusting                   /*!< Use module variation correcting LSC */
} cmc_sa_level;

/*!
 * \brief Light source enumeration
 */
typedef enum {
    cmc_light_source_none = 0,           /*!< Light source N/A */
    cmc_light_source_a,                  /*!< Incandescent / Tungsten */
    cmc_light_source_b,                  /*!< Obsolete. Direct sunlight at noon */
    cmc_light_source_c,                  /*!< Obsolete. Average / north sky daylight */
    cmc_light_source_d50,                /*!< Horizon light. ICC profile PCS */
    cmc_light_source_d55,                /*!< Mid-morning / mid-afternoon daylight */
    cmc_light_source_d65,                /*!< Noon daylight. Television, sRGB color space */
    cmc_light_source_d75,                /*!< North sky daylight */
    cmc_light_source_e,                  /*!< Equal energy */
    cmc_light_source_f1,                 /*!< Daylight fluorescent */
    cmc_light_source_f2,                 /*!< Cool white fluorescent */
    cmc_light_source_f3,                 /*!< White fluorescent */
    cmc_light_source_f4,                 /*!< Warm white fluorescent */
    cmc_light_source_f5,                 /*!< Daylight fluorescent */
    cmc_light_source_f6,                 /*!< Lite white fluorescent */
    cmc_light_source_f7,                 /*!< D65 simulator, daylight simulator */
    cmc_light_source_f8,                 /*!< D50 simulator, Sylvania F40 Design 50 */
    cmc_light_source_f9,                 /*!< Cool white deluxe fluorescent */
    cmc_light_source_f10,                /*!< Philips TL85, Ultralume 50 */
    cmc_light_source_f11,                /*!< Philips TL84, Ultralume 40 */
    cmc_light_source_f12                 /*!< Philips TL83, Ultralume 30 */
} cmc_light_source;
#define CMC_NUM_LIGHTSOURCES 20

/*!
 * \brief CMC Comment
 */
typedef struct
{
    ia_mkn_record_header header; /*!< Record header with Format ID: UInt8 (See AIQB_DataID), Name ID: cmc_name_id_comment (See cmc_name_id). */
    uint8_t project_id[16];      /*!< NULL terminated Project ID string. Date format: yymmddHHMMSSFFF. For example: 1202201823444. */
/*    uint8_t comment[];          / *!< Free C string text comment (NULL terminated). Must end at 64 bit alignment. * / */
} cmc_comment_t;
#define SIZEOF_CMC_COMMENT_T 24

/*!
 * \brief CMC General Data
 */
typedef struct
{
    ia_mkn_record_header header; /*!< Record header with Format ID: Uint16 (See AIQB_DataID) Name ID: cmc_name_id_general_data. (enum cmc_name_id). */
    uint16_t width;              /*!< Sensor native maximum width. */
    uint16_t height;             /*!< Sensor native maximum height. */
    uint16_t bit_depth;          /*!< Sensor native maximum bit depth (after sensor linearization). */
    uint16_t color_order;        /*!< Sensor color order in native orientation. */
    uint16_t bit_depth_packed;   /*!< Sensor native maximum bit depth (before sensor linearization = packed) . */
    uint8_t sve_pattern[16];     /*!< 4x4 pixel pattern. 0 means shortest exposure time and n the longest. */
} cmc_general_data_t;
#define SIZEOF_CMC_GENERAL_DATA_V100_T 16
#define SIZEOF_CMC_GENERAL_DATA_V101_T 18
#define SIZEOF_CMC_GENERAL_DATA_V102_T 34

/*!
 * \brief CMC Black Level
 * - Global black level compensation.
 */
typedef struct
{
    uint32_t exposure_time;              /*!< Exposure time. */
    uint32_t analog_gain;                /*!< Analog gain. */
    cmc_color_channels color_channels;     /*!< Color channel correction for given exposure time and analog gain. */
} cmc_black_level_lut_t;
#define SIZEOF_CMC_BLACK_LEVEL_LUT_T 16

typedef struct
{
    ia_mkn_record_header header;     /*!< Record header with Format ID: Custom (See AIQB_DataID) Name ID: cmc_name_id_black_level. (enum cmc_name_id). */
    uint32_t num_bl_luts;            /*!< Number of black level lookup tables (number of analog gain * number of exposures).  */
/*    cmc_black_level_lut_t bl_luts[]; / *!< Lookup tables for black level correction. * / */
} cmc_black_level_t;
#define SIZEOF_CMC_BLACK_LEVEL_T 12

/*!
 * \brief CMC Saturation Level
 * - Defined for each color channels.
 * - Absolute pixel values using native max bit depth.
 */
typedef struct
{
    ia_mkn_record_header header; /*!< Record header with Format ID: UInt16 (See AIQB_DataID) Name ID: cmc_name_id_saturation_level. (enum cmc_name_id). */
    uint16_t saturation_cc1;     /*!< Saturation level of 1st color channel. */
    uint16_t saturation_cc2;     /*!< Saturation level of 2nd color channel. */
    uint16_t saturation_cc3;     /*!< Saturation level of 3rd color channel. */
    uint16_t saturation_cc4;     /*!< Saturation level of 4th color channel. */
} cmc_saturation_level_t;
#define SIZEOF_CMC_SATURATION_LEVEL_T 16

/*!
 * \brief CMC Pixel Dynamic Range And Linearity
 */
typedef struct
{
    ia_mkn_record_header header; /*!< Record header with Format ID: Custom (See AIQB_DataID) Name ID: cmc_name_id_dynamic_range_and_linearity. (enum cmc_name_id). */
    uint32_t dynamic_range;      /*!< Pixel dynamic range in dB. */
    uint8_t num_linearity_cc1;   /*!< Number of points in color channel 1 linearity lookup table. */
    uint8_t num_linearity_cc2;   /*!< Number of points in color channel 2 linearity lookup table. */
    uint8_t num_linearity_cc3;   /*!< Number of points in color channel 3 linearity lookup table. */
    uint8_t num_linearity_cc4;   /*!< Number of points in color channel 4 linearity lookup table. */
/*    uint16_t lut_linearities[]; / *!< Linearity lookup table for color channels in order: Ch1, Ch2, Ch3 and Ch4. * / */
} cmc_linearity_t;
#define SIZEOF_CMC_LINEARITY_T 16

/*!
 * \brief CMC Module Sensitivity
 */
typedef struct
{
    ia_mkn_record_header header; /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_module_sensitivity (See cmc_name_id). */
    uint16_t base_iso;           /*!< Base ISO value of the camera module. */
} cmc_sensitivity_t;
#define SIZEOF_CMC_SENSITIVITY_T 10

/*!
 * \brief CMC Defect Pixels
 */
typedef struct
{
    ia_mkn_record_header header; /*!< Record header with Format ID: ??? (See AIQB_DataID), Name ID: cmc_name_id_defect_pixels (See cmc_name_id). */
    uint16_t not_defined;        /*!<  */
} cmc_defect_pixel_t;
#define SIZEOF_CMC_DEFECT_PIXEL_T 10

/*!
 * \brief CMC Noise
 * - signal dependent and signal independent noise models.
 * - Parametric model (a and b); variance as a function of relative pixel intensity.
 */
typedef struct
{
    ia_mkn_record_header header; /*!< Record header with Format ID: Float (See AIQB_DataID), Name ID: cmc_name_id_noise (See cmc_name_id). */
    float noise_model_a[2];      /*!< Linear signal dependent noise model. */
    float noise_model_b[3];      /*!< Quadratic signal independent noise model. */
} cmc_noise_t;
#define SIZEOF_CMC_NOISE_T 28

typedef struct
{
    uint16_t source_type;      /*!< Light source type (enum), e.g. Fluorescent. */
    uint16_t correction_level; /*!< Luminance correction level. */
    cie_coords_t cie_coords;   /*!< CIE x and y coordinates. */
/*    uint16_t lsc_grid[];       / *!< LSC Grid. * / */
} cmc_lsc_grid_t;
#define SIZEOF_CMC_LSC_GRID_T 8

/*!
 * \brief CMC_Lens Shading Correction
 * - Full native FOV
 * - Absolute gain values
 * - 100% color shading correction
 * - x% grid "divided by" NVM_calibration_grid (=ratio) - if no NVM use ones
 */
typedef struct
{
    ia_mkn_record_header header; /*!< Record header with Format ID: Custom (See AIQB_DataID), Name ID: cmc_name_id_lens_shading_correction (See cmc_name_id). */
    uint16_t lsc_level;          /*!< LSC level enumeration. */
    uint16_t num_grids;          /*!< Number of LSC grids. */
    uint16_t grid_width;         /*!< LSC Grid width. */
    uint16_t grid_height;        /*!< LSC Grid height. */
/*  cmc_lsc_grid_t lsc_grids[]; / *!< LSC grids. * / */
} cmc_lens_shading_t;
#define SIZEOF_CMC_LENS_SHADING_T 16

typedef struct
{
    uint16_t pair_index;       /*!< Calibration light source pair index. */
    uint16_t source_type;      /*!< Light source type (enum), e.g. Fluorescent. */
    uint16_t correction_level; /*!< Luminance correction level. */
    cie_coords_t cie_coords;   /*!< CIE x and y coordinates. */
/*    uint16_t lsc_grid[];       / *!< LSC Grid. * / */
} cmc_lsc_ratio_grid_t;
#define SIZEOF_CMC_LSC_RATIO_GRID_T 10

/*!
 * \brief Lens Shading Correction Ratio
 */
typedef struct
{
    ia_mkn_record_header header;      /*!< Record header with Format ID: Custom (See AIQB_DataID), Name ID: cmc_name_id_lens_shading_correction_ratio (See cmc_name_id). */
    uint16_t num_grids;               /*!< Number of LSC grids. */
    uint16_t grid_width;              /*!< LSC Grid width. */
    uint16_t grid_height;             /*!< LSC Grid height. */
/*    cmc_lsc_ratio_grid_t lsc_grids[]; / *!< LSC grids. * / */
} cmc_lens_shading_ratio_t;
#define SIZEOF_CMC_LENS_SHADING_RATIO_T 14

/*!
 * \brief CMC Geometric Distortion Correction
 */
typedef struct
{
    ia_mkn_record_header header; /*!< Record header with Format ID: Float (See AIQB_DataID), Name ID: cmc_name_id_geometric_distortion_correction (See cmc_name_id). */
    float gdck1;                 /*!<  1st order radial distortion coefficient. */
    float gdck2;                 /*!<  2nd order radial distortion coefficient. */
    float gdck3;                 /*!<  3rd order radial distortion coefficient. */
    float gdcp1;                 /*!<  1st order tangential distortion coefficient. */
    float gdcp2;                 /*!<  2nd order tangential distortion coefficient. */
} cmc_geometric_distortion_t;
#define SIZEOF_CMC_GEOMETRIC_DISTORTION_T 28

/*!
 * \brief CMC Optics and Mechanics
 */
typedef struct
{
    ia_mkn_record_header header;                         /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_optics_and_mechanics (See cmc_name_id). */
    uint8_t actuator;                                    /*!< Actuator type (enum). */
    uint8_t camera_module_orientation;                   /*!< Camera module orientation during the AF calibration (enum from Camera Module Orientation). */
    uint16_t camera_actuator_features;                   /*!< Camera features list (flags from cmc_camera_feature enumeration). */
    uint16_t nd_gain;                                    /*!< Neutral density filter gain. */
    uint16_t effect_focal_length;                        /*!< Effective Focal Length, (mm * 100). */
    uint16_t sensor_pix_size_v;                          /*!< Sensor pixel size Vertical, (um * 100). */
    uint16_t sensor_pix_size_h;                          /*!< Sensor pixel size Horizontal, (um * 100). */
    uint16_t sensor_width_pix_total;                     /*!< Sensor width in pixels, total. */
    uint16_t sensor_height_pix_total;                    /*!< Sensor height in pixels, total. */
    uint16_t lens_offset_up_to_horz;                     /*!< Lens displacement when module turns from Up to Horizontal, (um). */
    uint16_t lens_offset_horz_to_down;                   /*!< Lens displacement when module turns from Horizontal to Down, (um). */
    uint16_t range_inf_to_85mm;                          /*!< Optical range 'Inf - 8.5 cm', (um). */
    uint16_t range_inf_to_100mm;                         /*!< Optical range 'Inf - 10 cm', (um). */
    uint16_t range_inf_to_300mm;                         /*!< Optical range 'Inf - 30 cm', (um). */
    uint16_t range_inf_to_500mm;                         /*!< Optical range 'Inf - 50 cm', (um). */
    uint16_t range_inf_to_950mm;                         /*!< Optical range 'Inf - 95 cm', (um). */
    uint16_t range_inf_to_1200mm;                        /*!< Optical range 'Inf - 120 cm', (um). */
    uint16_t range_inf_to_hyperfocal;                    /*!< Optical range 'Inf - Hyperfocal', (um). */
    uint16_t range_inf_to_calibration_distance_far;      /*!< Optical range 'Inf - calibration distance far', (um). */
    uint16_t range_inf_to_calibration_distance_near;     /*!< Optical range 'Inf - calibration_distance_near', (um). */
    uint16_t range_inf_to_min_focusing_distance;         /*!< Optical range 'Inf - calibration_distance_near', (um). */
    uint16_t calibration_distance_far;                   /*!< Distance to supplier's FAR production calibration target, (cm). */
    uint16_t calibration_distance_near;                  /*!< Distance to supplier's NEAR production calibration target, (cm). */
    int16_t calibration_position_far;                    /*!< Supplier's FAR production calibration target in ACTUATOR UNITS, default (used in case NVM is not available).*/
    int16_t calibration_position_near;                   /*!< Supplier's NEAR production calibration target in ACTUATOR UNITS,  default (used in case NVM is not available).*/
    int32_t lens_range_limit;                            /*!< Maximum available value for the lens actuator. */
    int32_t lens_actuator_offset;                        /*!< Permanent offset to lens actuator values. */
    uint32_t lens_movement_time;                         /*!< Time (in us) needed to move the lens per single VC unit (if linear_lens_movement_time is 1). Total time needed per one lens move (if linear_lens_movement_time is 0)*/
    uint16_t min_focus_distance;                         /*!< Minimum focusing distance, (cm). */
    uint16_t num_apertures;                              /*!< Actual number of apertures, presented in camera. */
} cmc_optomechanics_t;
#define SIZEOF_CMC_OPTOMECHANICS    72

/*!
 * \brief CMC Module Spectral Response
 */
typedef struct
{
    ia_mkn_record_header header;          /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_module_spectral_response (See cmc_name_id). */
    uint16_t min_wavelength;              /*!< Minimum wave length. */
    uint16_t max_wavelength;              /*!< Maximum wave length. */
    uint16_t wavelength_sampling_rate;    /*!< Wave length sampling rate. */
/*    cmc_color_channels spectral_response[]; / *!< Module spectral response in order: Ch1,Ch2,Ch3 and Ch4. * / */
} cmc_spectral_response_t;
#define SIZEOF_CMC_SPECTRAL_RESPONSE 14

/*!
 * \brief Lightsource definition.
 */
typedef struct
{
    cie_coords_t cie_coords;           /*!< Light source CIE xy coordinates. */
    chromaticity_t chromaticity_response; /*!< Avg Chromaticity response for R/G anf B/G.*/
} cmc_lightsource_t;
#define SIZEOF_CMC_LIGHTSOURCE_T 8

/*!
 * \brief CMC Chromaticity Response
 */
typedef struct
{
    ia_mkn_record_header header;      /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_chromaticity_response (See cmc_name_id). */
    uint16_t num_lightsources;        /*!< Number of avg light sources. */
    uint16_t num_nvm_lightsources;    /*!< Number of nvm light sources. */
/*  cmc_lightsource_t lightsources[]; / *!< Lightsources in the order: avg, high and low. * / */
} cmc_chromaticity_response_t;
#define SIZEOF_CMC_CHROMATICITY_RESPONSE 12

/*!
 *  \brief Flash chromaticity responses for a point.
 */
typedef struct
{
    chromaticity_t flash_chromaticity_response;    /*!< Flash Chromaticity response, R/G and B/G */
} cmc_poly_point_t;
#define SIZEOF_CMC_POLY_POINT_T 4

/*!
 * \brief CMC Flash Chromaticity
 */
typedef struct
{
    ia_mkn_record_header header;        /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_flash_chromaticity (See cmc_name_id). */
    chromaticity_t flash_avg_chromaticity; /*!< Flash chromaticity in R/G, B/G plane. */
    uint16_t num_poly_points;           /*!< Number of points defining polygon. */
/*    cmc_poly_point_t poly_point[];    / *!< Flash Chromaticity Deviation in R/G, B/G plane. * / */
} cmc_flash_chromaticity_t;
#define SIZEOF_CMC_FLASH_CHROMATICITY_T 14

typedef struct
{
    cmc_light_source light_src_type; /*!< Light-source type. See enum cmc_light_source.  */
    chromaticity_t chromaticity;     /*!< Chromaticity (sensor) in R/G, B/G plane. */
    cie_coords_t cie_coords;      /*!< CIE x and y coordinates. */
    int32_t matrix_accurate[9];   /*!< 3x3 accurate CCM, each 3 consequtive elemets sum to 1. */
    int32_t matrix_preferred[9];  /*!< 3x3 preferred CCM, each 3 consequtive elemets sum to 1. */
} cmc_color_matrix_t;
#define SIZEOF_CMC_COLOR_MATRIX_T 84

/*!
 * \brief CMC Color matrices
 */
typedef struct
{
    ia_mkn_record_header header;         /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_color_matrices (See cmc_name_id). */
    uint16_t num_matrices;               /*!< Number of color matrices. */
/*    cmc_color_matrix_t color_matrices[]; / *!< Color matrices. * / */
} cmc_color_matrices_t;
#define SIZEOF_CMC_COLOR_MATRICES_T 10
/*!
 * \brief CMC NVM Info
 */
typedef struct
{
    ia_mkn_record_header header;      /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_nvm_info (See cmc_name_id). */
    uint16_t nvm_parser_version;      /*!< Parser version for current camera module type */
    uint16_t nvm_data_color_order;    /*!< NVM data color order */
    uint16_t nvm_data_orientation;    /*!< NVM data orientation */
} cmc_nvm_info_t;
#define SIZEOF_CMC_NVM_INFO_T 14

/*!
 * \brief CMC NVM Info v101
 */
typedef struct
{
    uint16_t nvm_scaling_method;        /*!< NVM table scaling method. */
} cmc_nvm_info_v101_t;
#define SIZEOF_CMC_NVM_INFO_V101_T   2

typedef struct
{
    cmc_nvm_info_t *cmc_nvm_info;           /*!< CMC NVM info data. */
    cmc_nvm_info_v101_t *cmc_nvm_info_v101; /*!< CMC NVM info data v101. */
} cmc_parsed_nvm_info_t;

/*!
 * \brief Analog gain to gain code mapping.
 * Gains must be defined in ascending order.
 */
typedef struct
{
    uint32_t gain;                 /*!< Gain in fixed point format (16bit integer part + 16bit fraction part). */
    uint32_t code;                 /*!< Code corresponding to gain. */
} cmc_analog_gain_pair_t;
#define SIZEOF_CMC_ANALOG_GAIN_PAIR_T 8

/*!
 * \brief Analog gain to gain code mapping of a segment.
 * Segments contain SMIA analog gain parameters. When ranges is defined, beginning is always inclusive and end exclusive.
 * For example analog gain 2.0 and ranges: [1.0, 2.0[; [2.0, 4.0[;. Analog gain 2.0 is not calculated from the first range parameters but from the second range.
 */
typedef struct
{
    uint32_t gain_begin;         /*!< Begin of gain of the segment (inclusive) in fixed point format (16bit integer part + 16bit fraction part). */
    uint32_t gain_end;           /*!< End of gain of the segment (exclusive) in fixed point format (16bit integer part + 16bit fraction part). */
    uint32_t code_min;           /*!< The minimum recommended setting for the analog gain control. */
    uint32_t code_max;           /*!< The maximum recommended setting for the analog gain control. */
    uint32_t code_step;          /*!< The precision of the analog gain control. */
    int16_t M0;                  /*!< Gain code M0 as in SMIA. */
    int16_t C0;                  /*!< Gain code C0 as in SMIA. */
    int16_t M1;                  /*!< Gain code M1 as in SMIA. */
    int16_t C1;                  /*!< Gain code C1 as in SMIA. */
} cmc_analog_gain_segment_t;
#define SIZEOF_CMC_ANALOG_GAIN_SEGMENT_T 28

/*!
 * \brief CMC Analog gain conversion types
 * Enum definses different analog gain conversion types.
 */
typedef enum
{
    cmc_analog_gain_conversion_type_none,    /*!< No analog gain conversion should be done. */
    cmc_analog_gain_conversion_type_segment, /*!< Segments contain SMIA compatible parameters for calculating register value for a certain range of analog gain. */
    cmc_analog_gain_conversion_type_pair,    /*!< Pairs contain analog gain value & corresponding register value. */
} cmc_analog_gain_conversion_type_t;

/*!
 * \brief CMC Analog gain conversion table
 * Analog gain can be represented with n amount of gain code (register value) pairs/segments.
 */
typedef struct
{
    ia_mkn_record_header header;         /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_analog_gain_conversion (See cmc_name_id). */
    uint16_t conversion_type;            /*!< Analog gain conversion type.  See cmc_analog_gain_conversion_type_t. */
    uint16_t reserved;                   /*!< Reserved due to byte alignment. */
    uint16_t num_segments;               /*!< Number of gain/code segments which describe the analog gain. */
    uint16_t num_pairs;                  /*!< Number of gain/code pairs which describe the analog gain. */
/*    cmc_analog_gain_segment_t gain_segments[]; */
/*    cmc_analog_gain_pair_t gain_pairs[]; */
} cmc_analog_gain_conversion_t;
#define SIZEOF_CMC_ANALOG_GAIN_CONVERSION_T 16

/*!
 * \brief CMC Digital gain conversion types
 * Enum definses different digital gain conversion types.
 */
typedef enum
{
    cmc_digital_gain_conversion_type_fixed_point, /*!< Digital gain is defined in fixed point format. */
    cmc_digital_gain_conversion_type_pair,        /*!< Pairs contain digital gain value & corresponding register value. */
} cmc_digital_gain_conversion_type_t;

/*!
 * \brief CMC digital gain limits and step.
 */
typedef struct
{
    ia_mkn_record_header header;         /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_digital_gain (See cmc_name_id). */
    uint16_t digital_gain_min;           /*!< The minimum valid limit of the digital gain control parameters in fixed point format (16bit integer part + 16bit fraction part). */
    uint16_t digital_gain_max;           /*!< The maximum valid limit of the digital gain control parameters in fixed point format (16bit integer part + 16bit fraction part). */
    uint8_t digital_gain_step_size;      /*!< Step size of digital gain code register value. */
    uint8_t digital_gain_fraction_bits;  /*!< Number of bits used for the fraction part of the 16 bit register value. */
/*    cmc_digital_gain_v102_t dg_v102; */
/*    cmc_analog_gain_pair_t dg_pairs[];*/
} cmc_digital_gain_t;
#define SIZEOF_CMC_DIGITAL_GAIN_T 14

/*!
 * \brief Defines additions to digital gain structure with gain/code pairs support.
 */
typedef struct
{
    uint16_t conversion_type;            /*!< Digital gain conversion type.  See cmc_digital_gain_conversion_type_t. */
    uint16_t num_pairs;                  /*!< Number of gain/code pairs which describe the digital gain. */
    uint16_t reserved;                   /*!< Not used at the moment. Reserved to ensure correct (32 bit) alignment. */
} cmc_digital_gain_v102_t;
#define SIZEOF_CMC_DIGITAL_GAIN_V102_T 6

/*!
 * \brief CMC geometric distortion correction (grid based)
 */
typedef struct
{
    ia_mkn_record_header header;         /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: CMC_GeometricDistortion2 (See cmc_name_id). */
    int16_t GDC_col_start;               /*!< Table X offset in pixels from left corner of the sensor maximum visible area e.g. If GDC_col_start=GDC_block_width*(-1)
                                            then GDC table offset is is one block left compared to the maximum visible sensor area. */
    int16_t GDC_row_start;               /*!< Table Y offset in pixels from upper corner of the sensor maximum visible area e.g. If GDC_row_start=GDC_block_height*(-1)
                                            then GDC table offset is is one block up compared to the maximum visible sensor area.  */
    int16_t GDC_grid_width;              /*!< Indicates number of grid vertices on the horizontal axis. */
    int16_t GDC_grid_height;             /*!< Indicates number of grid vertices on the vertical axis. */
    int16_t GDC_block_width;             /*!< Average width of the grid cell in pixel count. */
    int16_t GDC_block_height;            /*!< Average height of the grid cell in pixel count. */
    uint16_t nGrids;                     /*!< Number of LDC grids (focus positions). */
} cmc_geometric_distortion2_t;
#define SIZEOF_CMC_GEOMETRIC_DISTORTION2_T 22

/*!
 * \brief CMC geometric distortion correction grids
 */
typedef struct
{
    uint16_t focus_position;             /*!< Focus motor position in terms of those used by the sensor module.
                                            Range should be depicted from the cmc_name_id_optics_and_mechanics section in the CPFF.). */
    int32_t *x_deltas;                  /*!< Table of x-axis deltas of the grid points. The delta at each point represents the distortion
                                            that was done. Contains [GDC_grid_height  x GDC_grid_width] values. */
    int32_t *y_deltas;                  /*!< Table of y-axis deltas of the grid points. The delta at each point represents the distortion
                                            that was done. Contains [GDC_grid_height  x GDC_grid_width] values. */
} cmc_geometric_distortion2_grid_t;
#define SIZEOF_CMC_ANALOG_GAIN_PAIR_T 8

/*!
 * \brief CMC Sensor exposure registers ranges.
 */
typedef struct
{
    ia_mkn_record_header header;         /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_exposure_range (See cmc_name_id). */
    uint32_t coarse_integration_min;     /*!< Minimum sensor register value for coarse integration time. */
    uint32_t coarse_integration_max;     /*!< Maximum sensor register value for coarse integration time.*/
    uint32_t fine_integration_min;       /*!< Minimum sensor register value for fine integration time.*/
    uint32_t fine_integration_max;       /*!< Maximum sensor register value for fine integration time.*/
} cmc_exposure_range_t;
#define SIZEOF_CMC_EXPOSURE_RANGE_T 24

/*!
 * \brief CMC Multiple LED flash chromaticity.
 */
typedef struct
{
    uint16_t device_id;                    /*!< Flash device ID enumerator. */
    uint16_t num_poly_points;              /*!< Number of points defining polygon. */
    chromaticity_t flash_avg_chromaticity; /*!< Flash chromaticity in R/G, B/G plane. */
    cie_coords_t flash_avg_cie;            /*!< Flash chromaticity in CIE X, CIE Y plane. */
    uint16_t reserved[2];                  /*!< Reserved for future changes. Always 0 (for now).. */
    cmc_poly_point_t *poly_points;         /*!< Flash Chromaticity Deviation in R/G, B/G plane. */
} cmc_flash_device_t;
#define SIZEOF_CMC_FLASH_DEVICE_T 16     /*!< Size of the structure without pointers. Used in copying data from CPF to this structure. */

typedef struct
{
    ia_mkn_record_header header;         /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_multi_led_flash_chromaticity (See cmc_name_id). */
    uint16_t max_flash_output;           /*!< Flash maximum energy output for full power, lumen per second [lm/s] */
    uint16_t multi_led_flash_mode;       /*!< Reserved for future changes. Always 0 (for now). */
    uint16_t reserved;                   /*!< Reserved for future changes. Always 0 (for now). */
    uint16_t num_flash_devices;          /*!< Number of flash devices/LEDs. */
    cmc_flash_device_t *flash_devices;   /*!< Data of all flash devices. */
} cmc_multi_led_flash_t;
#define SIZEOF_CMC_MULTI_LED_FLASH_T 16  /*!< Size of the structure without pointers. Used in copying data from CPF to this structure. */

typedef struct
{
    ia_mkn_record_header header;         /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_sensor_metadata (See cmc_name_id). */
    uint16_t total_num_of_cfg_blocks;    /*!< Total number of config blocks (ia_emd_config_block_t). */
    uint16_t total_num_of_data_blocks;   /*!< Total number of decoded data blocks (ia_emd_decoded_block_t). */
    uint8_t num_of_exposure_sets;        /*!< Number of exposure sets in sensor embedded data. */
    uint8_t num_of_color_channels;       /*!< Number of color channels in sensor embedded data. */
    uint8_t max_num_of_faces;            /*!< Maximum number of faces in external ISP embedded data. */
    uint8_t reserved;                    /*!< Reserved. */
    /* ia_emd_config_block_t cfg_blocks[];*/   /*!< Configuration blocks array. */
} cmc_emd_decoder_config_t;
#define SIZEOF_CMC_EMD_DECODER_CONFIG_T  16 /*!< Size of cmc_emd_decoder_config_t without pointer. Used in copying data from CPF to this structure. */

typedef struct
{
    cmc_comment_t *cmc_comment;
    uint8_t *comment;
} cmc_parsed_comment_t;

typedef struct
{
    cmc_black_level_t *cmc_black_level;
    cmc_black_level_lut_t *cmc_black_level_luts;
} cmc_parsed_black_level_t;

typedef struct
{
    cmc_linearity_t *cmc_linearity;
    uint16_t *cmc_linearity_lut;
} cmc_parsed_linearity_t;

typedef struct
{
    cmc_lens_shading_t *cmc_lens_shading;
    cmc_lsc_grid_t *cmc_lsc_grids;
    uint16_t *lsc_grids;
    chromaticity_t *cmc_lsc_rg_bg_ratios;  /* Sensor R/G B/G ratios, available in
                                               cmc_name_id_lens_shading_correction v101. */
} cmc_parsed_lens_shading_t;

typedef struct
{
    cmc_lens_shading_ratio_t *cmc_lens_shading_ratio;
    cmc_lsc_ratio_grid_t *cmc_lsc_ratio_grids;
    uint16_t *lsc_grids;
} cmc_parsed_lens_shading_ratio_t;

typedef enum
{
    cmc_aperture_type_fixed = 0,          /*!< Aperture is fixed. */
    cmc_aperture_type_dc_iris,            /*!< Aperture is controlled by DC-iris. */
    cmc_aperture_type_p_iris,             /*!< Aperture is controlled by P-iris. */
} cmc_aperture_type_t;

typedef struct
{
    int16_t cmc_dac_min;        /*!< Minimum value of the DA-converter (will close the iris). */
    int16_t cmc_dac_hold;       /*!< A value for DA-converter that will hold the iris ("barely open or barely close"). */
    int16_t cmc_dac_max;        /*!< Maximum value of the DA-converter (will open the iris). */
} cmc_dc_iris_dac_t;

typedef struct
{
    cmc_optomechanics_t *cmc_optomechanics;
    uint16_t *lut_apertures;
    uint16_t aperture_type;         /*!< Aperture type control of type cmc_aperture_type_t, v101 and onwards. */
    cmc_dc_iris_dac_t dc_iris_dac;  /*!< DAC values for DC-iris, v102 and onwards. */
} cmc_parsed_optics_t;

typedef struct
{
    cmc_spectral_response_t *cmc_spectral_response;
    cmc_color_channels *spectral_responses;
} cmc_parsed_spectral_response_t;

/*!
 * \brief CMC Gamut.
 */
typedef struct
{
    uint16_t light_source;               /*!< Light source type (0 if not known). */
    uint16_t r_per_g;                    /*!< Gamut achromatic point R per G (white balance gains for given gamut).*/
    uint16_t b_per_g;                    /*!< Gamut achromatic point R per G (white balance gains for given gamut).*/
    uint16_t CIE_x;                      /*!< Illumination CIE x coordinate.*/
    uint16_t CIE_y;                      /*!< Illumination CIE x coordinate.*/
    uint16_t size;                       /*!< Size of the gamut tables.*/
    uint16_t *gamut_r_per_g;             /*!< Illumination gamut (convex hull). R per g points in clockwise order*/
    uint16_t *gamut_b_per_g;             /*!< Illumination gamut (convex hull). R per g points in clockwise order*/
} cmc_gamut_t;
#define SIZEOF_CMC_GAMUT_T 12

typedef struct
{
    uint16_t num_illumination_gamuts;   /*!< Number of illumination gamuts */
    cmc_gamut_t *cmc_gamut;
} cmc_chromaticity_response_v101_t;
#define SIZEOF_CMC_CHROMATICITY_RESPONSE_V101 2

typedef struct
{
    cmc_chromaticity_response_t *cmc_chromaticity_response;
    cmc_chromaticity_response_v101_t *cmc_chromaticity_response_v101;
    cmc_lightsource_t *cmc_lightsources_avg;
    cmc_lightsource_t *cmc_lightsources_hi;
    cmc_lightsource_t *cmc_lightsources_lo;
    cmc_lightsource_t *cmc_lightsources_nvm;
} cmc_parsed_chromaticity_response_t;

typedef struct
{
    cmc_flash_chromaticity_t *cmc_flash_chromaticity;
    cmc_poly_point_t *cmc_poly_points;
} cmc_parsed_flash_chromaticity_t;

typedef enum
{
    ccm_estimate_method_bypass,     /*!< Return unity CCM matrix. */
    ccm_estimate_method_wp,         /*!< Return CCM matrix using only white point estimate from AWB. */
    ccm_estimate_method_wp_sa,      /*!< Return CCM matrix using both white point estimate from AWB and Shading Adaptor Results. */
} ccm_estimate_method_t;

typedef struct
{
    cmc_color_matrices_t *cmc_color_matrices;
    cmc_color_matrix_t *cmc_color_matrix;
    uint16_t *ccm_estimate_method;              /*!< ccm_estimate_method for interpolation -> ccm_estimate_method_t */
} cmc_parsed_color_matrices_t;

typedef struct
{
    cmc_analog_gain_conversion_t *cmc_analog_gain_conversion;
    cmc_analog_gain_segment_t *cmc_analog_gain_segments;
    cmc_analog_gain_pair_t *cmc_analog_gain_pairs;
} cmc_parsed_analog_gain_conversion_t;

typedef struct
{
    cmc_digital_gain_t *cmc_digital_gain;
    cmc_digital_gain_v102_t *cmc_digital_gain_v102;
    cmc_analog_gain_pair_t *cmc_digital_gain_pairs;
} cmc_parsed_digital_gain_t;

typedef struct
{
    cmc_geometric_distortion2_t *geometric_distortion2;
    cmc_geometric_distortion2_grid_t *gdc_grids;
} cmc_parsed_geometric_distortion2_t;

/*!
 * \brief CMC advanced color matrix info
 */
typedef struct
{
    ia_mkn_record_header header;         /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: CMC_GeometricDistortion2 (See cmc_name_id). */
    uint16_t light_sources_count;        /*!< Number of light sources. */
    uint16_t sector_count;               /*!< Number of color matrix sectors.  */
} cmc_advanced_color_matrix_info_t;
#define SIZEOF_CMC_ADVANCED_COLOR_MATRIX_INFO_T 12

/*!
 * \brief CMC color matrix
 */
typedef struct
{
    float color_matrix[9];                 /*!< 3x3 accurate CCM, each consequtive elements sum to 1. */
} cmc_acm_color_matrix_t;
#define SIZEOF_CMC_ACM_COLOR_MATRIX_T 36

/*!
 * \brief CMC advanced color matrices info for light sources
 */
typedef struct
{
    uint32_t src_type;                   /*!< Light source type (enum), e.g. Fluorescent. */
    float chromaticity[2];               /*!< Chromaticity (sensor) in R/G, B/G plane. */
    float src_cie_xy[2];                 /*!< CIE x and y coordinates. */
} cmc_acm_color_matrices_info_t;
#define SIZEOF_CMC_ACM_COLOR_MATRICES_INFO_T 20

typedef struct
{
    cmc_acm_color_matrices_info_t *color_matrices_info;         /*! < Information for Color matrices. */
    cmc_acm_color_matrix_t traditional_color_matrix;            /*! < Color matrix optimized using all sectors. */
    cmc_acm_color_matrix_t *advanced_color_matrices;            /*! < Array of color matrices. Array size is sector_count */
} cmc_parsed_advanced_color_matrices_ls_t;

typedef struct
{
    cmc_advanced_color_matrix_info_t *cmc_advanced_color_matrix_info;
    uint32_t *hue_of_sectors;                                                       /*! < Starting hue angle array of sectors. Array size is sector_count. */
    cmc_parsed_advanced_color_matrices_ls_t *cmc_parsed_advanced_color_matrices_ls; /*! < Array of color matrices
                                                                                          for different light sources.
                                                                                          Array size is light_sources_count. */
} cmc_parsed_advanced_color_matrix_t;

/*!
* \brief CMC HDR Parameters
*/
typedef struct
{
    ia_mkn_record_header header;    /*!< Record header with Format ID: Uint16 (See AIQB_DataID) Name ID: cmc_name_id_hdr. */
    float hdr_exposure_ratio_min;   /*!< Minimum HDR exposure ratio between different exposures. */
    float hdr_exposure_ratio_max;   /*!< Maximum HDR exposure ratio between different exposures. */
}
cmc_parsed_hdr_parameters_t;
#define SIZEOF_CMC_HDR_T 16

typedef enum
{
    ir_correction_level_bypass,     /*!< Set PA IR Weight grid pointer as NULL. */
    ir_correction_level_wp,         /*!< Interpolate IR Weight grid using only white point estimate from AWB. */
} ir_correction_level_t;

/*!
* \brief CMC IR Weight Grids Parameters
*/
typedef struct
{
    ia_mkn_record_header header;    /*!< Record header with Format ID: Uint16 (See AIQB_DataID) Name ID: cmc_name_id_hdr. */
    int8_t grid_indices[16];        /*!< IR Weight grid indices showing which tables in the structure holds information. -1 means no correction for that pixel */
    uint16_t num_light_sources;     /*!< Number of light sources for which IR Weight Grids are characterized . */
    uint16_t num_grids;             /*!< Number of grids per light source. This depends on the CFA type and non -1 elements in  . */
    uint16_t grid_width;            /*!< IR Weight Grid width. */
    uint16_t grid_height;           /*!< IR Weight Grid height. */
}
cmc_ir_weight_info_t;
#define SIZEOF_CMC_IR_WEIGHT_INFO_T 32


typedef struct
{
    cmc_light_source source_type;    /*!< Light-source type. See enum cmc_light_source.  */
    float chromaticity[2];          /*!< Chromaticity (sensor) in R/G, B/G plane. */
    float cie_coords[2];            /*!< CIE x and y coordinates. */
} cmc_ir_weight_grid_info_t;
#define SIZEOF_CMC_IR_WEIGHT_GRID_INFO_T 20

typedef struct
{
    cmc_ir_weight_grid_info_t *ir_weight_grid_info;     /*!< IR Weight Grid Info. */
    uint16_t *ir_weight_grid;        /*!< IR Weight Grids for all channels. */
}
cmc_ir_weight_grids_t;

typedef struct
{
    cmc_ir_weight_info_t *ir_weight_info;       /*!< IR Weight Info . */
    cmc_ir_weight_grids_t *ir_weight_grids;     /*!< IR Weight Grids. */
}
cmc_parsed_ir_weight_t;

/* CMC structures with version ID >= v200 */

typedef struct
{
    cmc_light_source source_type;         /*!< Light source type (enum), e.g. Fluorescent. */
    cmc_chromaticity chromaticity;        /*!< Chromaticity (sensor) in R/G, B/G plane. */
    cmc_cie_coords cie_coords;            /*!< CIE x and y coordinates. */
    cmc_color_matrix traditional;         /*!< Color matrix optimized using all sectors. */
    cmc_color_matrix *advanced;           /*!< Array of color matrices (as defined by num_sectors). */
} cmc_advanced_color_matrices;

typedef struct
{
    ia_mkn_record_header header;          /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_advanced_color_matrices (See cmc_name_id). */
    unsigned short num_light_srcs;        /*!< Number of light sources. */
    unsigned short num_sectors;           /*!< Number of color matrix sectors.  */
    cmc_advanced_color_matrices *acms;    /*!< Advanced color matrices for all light sources (as defined by num_light_srcs). */
    unsigned int *hue_of_sectors;         /*!< Starting hue angle array of sectors (as defined by num_sectors). */
} cmc_advanced_color_matrix_correction;

typedef struct
{
    cmc_light_source source_type;         /*!< Light source type. */
    float chromaticity_i_per_g;           /*!< Sensor Ir/G ratio. */
    cmc_chromaticity chromaticity;        /*!< Sensor R/G B/G ratios. */
    cmc_cie_coords cie_coords;            /*!< CIE x and y coordinates. */
    uint16_t *grids[4][4];                /*!< IR Grids for all color channels (as defined by grid_indices). */
} cmc_ir_grid;

typedef struct
{
    ia_mkn_record_header header;          /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_infrared_correction (See cmc_name_id). */
    char grid_indices[4][4];              /*!< IR grid indices showing which tables in the structure holds information. -1 means no correction. */
    unsigned short num_light_srcs;        /*!< Number of light sources. */
    unsigned short grid_width;            /*!< IR Grid width. */
    unsigned short grid_height;           /*!< IR Grid height. */
    cmc_ir_grid *ir_grids;                /*!< IR grids for all light sources (as defined by num_light_srcs). */
} cmc_infrared_correction;

typedef struct
{
    cmc_light_source source_type;         /*!< Light source type. */
    float correction_level;               /*!< Luminance correction level. */
    cmc_chromaticity chromaticity;        /*!< Sensor R/G B/G ratios. */
    cmc_cie_coords cie_coords;            /*!< CIE x and y coordinates. */
    uint16_t *grids[4][4];                /*!< LSC Grids for all color channels (as defined by grid_indices). */
} cmc_lsc_grid;

typedef struct
{
    ia_mkn_record_header header;          /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_lens_shading_correction_4x4 (See cmc_name_id). */
    char grid_indices[4][4];              /*!< LSC grid indices showing which tables in the structure holds information. -1 means no correction. */
    unsigned short num_light_srcs;        /*!< Number of light sources. */
    unsigned short grid_width;            /*!< LSC Grid width. */
    unsigned short grid_height;           /*!< LSC Grid height. */
    cmc_lsc_grid *lsc_grids;              /*!< LSC grids for all light sources (as defined by num_light_srcs). */
} cmc_lens_shading_correction;

typedef struct
{
    ia_mkn_record_header header;          /*!< Record header with Format ID: UInt16 (See AIQB_DataID), Name ID: cmc_name_id_lateral_chromatic_aberration_correction (See cmc_name_id). */
    cmc_coords optical_center;            /*!< Optical center distance from the left uppermost corner of the sensor (in native sensors resolution). This the location where no lateral chromatic aberration is present. [x, y]. */
    uint16_t grid_width;                  /*!< Width of the grid. */
    uint16_t grid_height;                 /*!< Height of the grid. */
    uint16_t cell_size_x;                 /*!< X-dimension distance (in pixels) between the grid points (lca_grid_*). Guaranteed to be 2^n. This is always in respect to the maximum native sensor width defined in ID2 (General Data). All adaptations to other resolutions is done in the imaging system code in the device in the runtime.*/
    uint16_t cell_size_y;                 /*!< Same as above but y-dimension */
    float *lca_grid_red_x;                /*!< Amount of absolute lateral chromatic aberration in horizontal direction in each of the grid locations for red pixels, in respect to the green pixel. The grid is evenly spaced over the native maximum sensor resolution and maximum field of view. 0 means no aberration, -x means shifted to left, +x mean shifted to right. Grid is aligned to the left uppermost corner of the maximum focal plane. This, as all other CMC tables, are in respect to the maximum native image sensor resolution. All adaptations to other resolutions is done in the imaging system code in the device in the runtime. */
    float *lca_grid_red_y;                /*!< Same as above but in vertical direction. */
    float *lca_grid_blue_x;               /*!< Same as above but for horizontal and blue pixel. */
    float *lca_grid_blue_y;               /*!< Same as above but in vertical direction. */
} cmc_lateral_chromatic_aberration_correction;

/*!
 * \brief Parsed CMC structure.
 * Parser will fill the pointers in this structure so that data can be accessed more easily.
 */
typedef struct
{
    cmc_parsed_comment_t cmc_parsed_comment;                               /* 0 */
    cmc_general_data_t *cmc_general_data;                                  /* 8 */
    cmc_parsed_black_level_t cmc_parsed_black_level;                       /* 12 */
    cmc_saturation_level_t *cmc_saturation_level;                          /* 20 */
    cmc_parsed_linearity_t cmc_parsed_linearity;                           /* 24 */
    cmc_sensitivity_t *cmc_sensitivity;                                    /* 32 */
    cmc_defect_pixel_t *cmc_defect_pixel;                                  /* 36 */
    cmc_noise_t *cmc_noise;                                                /* 40 */
    cmc_parsed_lens_shading_t cmc_parsed_lens_shading;                     /* 44 */
    cmc_parsed_lens_shading_ratio_t cmc_parsed_lens_shading_ratio;         /* 56 */
    cmc_geometric_distortion_t *cmc_geometric_distortion;                  /* 68 */
    cmc_parsed_optics_t cmc_parsed_optics;                                 /* 72 */
    cmc_parsed_spectral_response_t cmc_parsed_spectral_response;           /* 80 */
    cmc_parsed_chromaticity_response_t cmc_parsed_chromaticity_response;   /* 88 */
    cmc_parsed_flash_chromaticity_t cmc_parsed_flash_chromaticity;         /* 108 */
    cmc_parsed_nvm_info_t cmc_parsed_nvm_info;                             /* 114 */
    cmc_parsed_color_matrices_t cmc_parsed_color_matrices;                 /* */
    cmc_parsed_analog_gain_conversion_t cmc_parsed_analog_gain_conversion;
    cmc_parsed_digital_gain_t cmc_parsed_digital_gain;
    cmc_parsed_geometric_distortion2_t cmc_parsed_geometric_distortion2;
    cmc_exposure_range_t *cmc_exposure_range;
    cmc_multi_led_flash_t *cmc_multi_led_flashes;
    cmc_emd_decoder_config_t *cmc_emd_decoder_config;
    cmc_parsed_advanced_color_matrix_t cmc_parsed_advanced_color_matrix;
    cmc_parsed_hdr_parameters_t *cmc_parsed_hdr_parameters;
    cmc_parsed_ir_weight_t cmc_parsed_ir_weight;
} ia_cmc_t;

#ifdef __cplusplus
}
#endif

#endif /* IA_CMC_TYPES_H_ */
