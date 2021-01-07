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
 * \file ia_aiq_types.h
 * \brief Definitions of input/output structures of the Intel 3A library.
 */

#ifndef _IA_AIQ_TYPES_H_
#define _IA_AIQ_TYPES_H_

#include "ia_types.h"
#include "ia_cmc_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_COLOR_CONVERSION_MATRIX 3
typedef struct ia_aiq_t ia_aiq;

/*!
 * \brief Target (frame use) for the analysis algorithms calculations.
 */
typedef enum
{
    ia_aiq_frame_use_preview,
    ia_aiq_frame_use_still,
    ia_aiq_frame_use_continuous,
    ia_aiq_frame_use_video,
} ia_aiq_frame_use;

/*!
 * \brief Camera orientations.
 */
typedef enum
{
    ia_aiq_camera_orientation_unknown,      /*!< Orientation not known. */
    ia_aiq_camera_orientation_rotate_0,     /*!< Non-rotated landscape. */
    ia_aiq_camera_orientation_rotate_90,    /*!< Portrait i.e. rotated 90 degrees clockwise. */
    ia_aiq_camera_orientation_rotate_180,   /*!< Landscape (upside down) i.e. rotated 180 degrees clockwise. */
    ia_aiq_camera_orientation_rotate_270    /*!< Portrait (upside down) i.e. rotated 270 degrees clockwise. */
} ia_aiq_camera_orientation;

/*!
 * \brief AEC flicker reduction modes.
 */
typedef enum
{
    ia_aiq_ae_flicker_reduction_off,     /*!< Disables flicker detection and reduction. */
    ia_aiq_ae_flicker_reduction_50hz,    /*!< Manual flicker reduction for 50Hz mains frequency. */
    ia_aiq_ae_flicker_reduction_60hz,    /*!< Manual flicker reduction for 60Hz mains frequency. */
    ia_aiq_ae_flicker_reduction_auto,    /*!< Detects flicker frequency and applies detected reduction. */
    ia_aiq_ae_flicker_reduction_detect,  /*!< Detects only flicker frequency but doesn't apply reduction. */
} ia_aiq_ae_flicker_reduction;

/*!
 * \brief AEC operation modes.
 */
typedef enum {
    ia_aiq_ae_operation_mode_automatic,         /*!< Automatic mode. */
    ia_aiq_ae_operation_mode_long_exposure,     /*!< AEC produces exposure parameters with long exposure (low light & static) scene. */
    ia_aiq_ae_operation_mode_action,            /*!< AEC produces exposure parameters for fast moving scene. */
    ia_aiq_ae_operation_mode_video_conference,  /*!< AEC produces exposure parameters which can be used in video conferencing scene. */
    ia_aiq_ae_operation_mode_production_test,   /*!< AEC produces exposure parameters which are used in production test environment. */
    ia_aiq_ae_operation_mode_ultra_low_light,   /*!< AEC produces exposure parameters which are used in ultra low light scene. */
    ia_aiq_ae_operation_mode_fireworks,         /*!< AEC produces exposure parameters which are used in fireworks scene. */
    ia_aiq_ae_operation_mode_hdr,               /*!< AEC produces exposure parameters which are used for HDR imaging (vHDR or exposure bracketing). */
    ia_aiq_ae_operation_mode_custom_1,          /*!< AEC produces exposure parameters for a specialized custom scene 1. */
    ia_aiq_ae_operation_mode_custom_2,          /*!< AEC produces exposure parameters for a specialized custom scene 2. */
    ia_aiq_ae_operation_mode_custom_3,          /*!< AEC produces exposure parameters for a specialized custom scene 3. */
} ia_aiq_ae_operation_mode;

/*!
 * \brief AEC metering modes.
 */
typedef enum {
    ia_aiq_ae_metering_mode_evaluative,  /*!< Exposure is evaluated from the whole frame. */
    ia_aiq_ae_metering_mode_center,      /*!< Exposure is evaluated center weighted. */
} ia_aiq_ae_metering_mode;

/*!
 * \brief AEC priority modes.
 */
typedef enum {
    ia_aiq_ae_priority_mode_normal,    /*!< All areas are equally important. */
    ia_aiq_ae_priority_mode_highlight, /*!< Highlights must be preserved even if it means that dark parts become very dark. */
    ia_aiq_ae_priority_mode_shadow,    /*!< Shadow areas are more important. */
} ia_aiq_ae_priority_mode;

/*!
 * \brief AEC exposure distribution priority modes
 *
 * This enumeration values are used to control distribution of AEC exposure parameters. For example in some situation user may want to keep
 * aperture at smallest value (in order to have large DOF) as long as possible in expense of motion blur (caused by long exposure time) and
 * noise (caused by high ISO).
 *
 * Note. Manual priority modes as understood by SLRs are achieved by using manual exposure parameters when running AEC:
 * Shutter priority: Set manual_exposure_time_us in ia_aiq_ae_input_params.
 * ISO priority: Set manual_iso in ia_aiq_ae_input_params.
 * Aperture priority: Set manual_aperture_fn in ia_aiq_ae_input_params.
 */
typedef enum
{
    ia_aiq_ae_exposure_distribution_auto,     /*!< AEC internally prioritizes between exposure time, aperture and ISO when calculating distribution. */
    ia_aiq_ae_exposure_distribution_shutter,  /*!< AEC tries to keep the exposure time at minimum until ISO and aperture are at maximum. */
    ia_aiq_ae_exposure_distribution_iso,      /*!< AEC tries to keep the ISO at minimum until exposure time and aperture are at maximum. */
    ia_aiq_ae_exposure_distribution_aperture, /*!< AEC tries to keep the aperture at minimum until exposure time and ISO are at maximum. */
} ia_aiq_ae_exposure_distribution_priority;

/*!
 * \brief AEC feature setting.
 */
typedef enum {
    ia_aiq_ae_feature_setting_tuning,     /*!< Feature setting is taken from tuning data. */
    ia_aiq_ae_feature_setting_disabled,   /*!< Feature setting is disabled. */
    ia_aiq_ae_feature_setting_enabled,    /*!< Feature setting is enabled. */
} ia_aiq_ae_feature_setting;


/*!
 * \brief Autofocus states
 */
typedef enum
{
    ia_aiq_af_status_idle,               /*!< Focus is idle */
    ia_aiq_af_status_local_search,       /*!< Focus is in local search state */
    ia_aiq_af_status_extended_search,    /*!< Focus is in extended search state */
    ia_aiq_af_status_success,            /*!< Focus has succeeded */
    ia_aiq_af_status_fail,               /*!< Focus has failed */
    ia_aiq_af_status_depth_search        /*!< Focus in depth search mode */
} ia_aiq_af_status;

/*!
 * \brief Action for the lens driver
 */
typedef enum
{
    ia_aiq_lens_driver_action_none,
    ia_aiq_lens_driver_action_move_to_unit,
    ia_aiq_lens_driver_action_move_by_units
} ia_aiq_lens_driver_action;

/*!
 * \brief Autofocus modes
 */
typedef enum
{
    ia_aiq_af_operation_mode_auto,                 /*!< Auto mode */
    ia_aiq_af_operation_mode_infinity,             /*!< Inifinity mode */
    ia_aiq_af_operation_mode_hyperfocal,           /*!< Hyperfocal mode */
    ia_aiq_af_operation_mode_manual,               /*!< Manual mode */
    ia_aiq_af_operation_mode_production_test,      /*!< Production test mode. */
    ia_aiq_af_operation_mode_depth_map,            /*!< Depth-map generation mode. */
    ia_aiq_af_operation_mode_depth,                /*!< Automatic focusing based on depth measurements only. */
    ia_aiq_af_operation_mode_contrast              /*!< Automatic focusing based on contrast measurements only. */
} ia_aiq_af_operation_mode;

/*!
 * \brief Autofocus range
 */
typedef enum
{
    ia_aiq_af_range_normal,                        /*!< Normal range */
    ia_aiq_af_range_macro,                         /*!< Macro range */
    ia_aiq_af_range_extended,                      /*!< Extended/full range */
} ia_aiq_af_range;


/*!
 * \brief Autofocus metering modes
 */
typedef enum
{
    ia_aiq_af_metering_mode_auto,                      /*!< Auto metering mode */
    ia_aiq_af_metering_mode_touch                      /*!< Touch metering mode */
} ia_aiq_af_metering_mode;

/*!
 * \brief Lens actuator status
 */
typedef enum
{
    ia_aiq_lens_status_stopped,                    /*!< Lens has not moved during the frame integration*/
    ia_aiq_lens_status_moving                      /*!< Lens has been moving during the frame integration */
} ia_aiq_lens_status;

/*!
 * \brief Action for the manual focus
 */
typedef enum
{
    ia_aiq_manual_focus_action_none,                /*!< No action for the manual focus is required */
    ia_aiq_manual_focus_action_set_distance,        /*!< Set manual focus distance */
    ia_aiq_manual_focus_action_set_lens_position,   /*!< Set manual lens position */
    ia_aiq_manual_focus_action_set_focal_distance   /*!< Set manual focal distance in um (distance between the lens and the sensor plane, e.g. 4390) */
} ia_aiq_manual_focus_action;

/*!
 * \brief Focus bracketing mode.
 */
typedef enum
{
    ia_aiq_af_bracket_mode_symmetric,        /*!< Symmetric focus bracketing around the reference lens position*/
    ia_aiq_af_bracket_mode_towards_near,     /*!< One side focus bracketing. Images are taken towards NEAR end (macro) */
    ia_aiq_af_bracket_mode_towards_far       /*!< One side focus bracketing. Images are taken towards FAR end (infinity)*/
} ia_aiq_af_bracket_mode;

/*!
 * \brief Detected scene mode.
 */
typedef enum
{
    ia_aiq_scene_mode_none                = 0,
    ia_aiq_scene_mode_close_up_portrait   = (1 << 0),
    ia_aiq_scene_mode_portrait            = (1 << 1),
    ia_aiq_scene_mode_lowlight_portrait   = (1 << 2),
    ia_aiq_scene_mode_low_light           = (1 << 3),
    ia_aiq_scene_mode_action              = (1 << 4),
    ia_aiq_scene_mode_backlight           = (1 << 5),
    ia_aiq_scene_mode_landscape           = (1 << 6),
    ia_aiq_scene_mode_document            = (1 << 7),
    ia_aiq_scene_mode_firework            = (1 << 8),
    ia_aiq_scene_mode_lowlight_action     = (1 << 9),
    ia_aiq_scene_mode_baby                = (1 << 10),
    ia_aiq_scene_mode_barcode             = (1 << 11)
} ia_aiq_scene_mode;

/*!
* \brief Mode for calculating AE bracketing.
*/
typedef enum
{
    ia_aiq_bracket_mode_none,             /*!< No bracketing used. */
    ia_aiq_bracket_mode_ull  = (1 << 0),  /*!< Ultra Low Light bracketing used. */
    ia_aiq_bracket_mode_hdr  = (1 << 1)   /*!< High Dynamic Range bracketing used. */
} ia_aiq_bracket_mode;

/*!
 * \brief Manual focus parameters.
 */
typedef struct
{
    unsigned int manual_focus_distance;               /*!< Manual focus distance in mm*/
    int manual_lens_position;                         /*!< Manual lens position */
    unsigned int manual_focal_distance;               /*!< Manual focal_distance in um (e.g. 4390) */
    ia_aiq_manual_focus_action manual_focus_action;   /*!< Manual focus action */
} ia_aiq_manual_focus_parameters;

/*!
 * \brief Exposure parameters in terms of generic units.
 * Structure can be used as input or output from AEC.
 */
typedef struct
{
    int exposure_time_us;          /*!< Exposure time in microseconds, -1 if N/A. */
    float analog_gain;             /*!< Analog gain as a multiplier (e.g. 1.0), -1.0 if N/A. */
    float digital_gain;            /*!< Digital gain as a multiplier (e.g. 1.0), -1.0 if N/A. */
    float aperture_fn;             /*!< f-number of aperture (e.g. 2.8), -1.0 for N/A. TODO: Move to ia_aiq_aperture_control structure. */
    int total_target_exposure;     /*!< Total exposure ie. combination of Et, Ag, Dg, Aperture gain and ND gain, -1 if N/A. */
    bool nd_filter_enabled;        /*!< true or false, false for N/A. */
    int iso;                       /*!< ISO value corresponding to the analog gain. -1 if N/A. */
} ia_aiq_exposure_parameters;

/*!
 * \brief Exposure parameters in terms of sensor units.
 * Structure can be used as input or output from AEC.
 */
typedef struct
{
    unsigned short fine_integration_time;         /*!< Integration time specified as a number of pixel clocks added on top of coarse_integration_time. */
    unsigned short coarse_integration_time;       /*!< Integration time specified in multiples of pixel_periods_per_line.*/
    unsigned short analog_gain_code_global;       /*!< Global analog gain code. */
    unsigned short digital_gain_global;           /*!< Global digital gain code. */
    unsigned short line_length_pixels;            /*!< The number of pixels in one row. This includes visible lines and horizontal blanking lines. */
    unsigned short frame_length_lines;            /*!< The number of complete lines (rows) in the output frame. This includes visible lines and vertical blanking lines. */
} ia_aiq_exposure_sensor_parameters;

/*!
 * \brief Exposure related restrictions and constants in terms of sensor units.
 * Camera driver fills and updates these parameters whenever they are changed (for example in mode change).
 */
typedef struct
{
    float pixel_clock_freq_mhz;                        /*!< Video timing pixel clock frequency. */
    unsigned short pixel_periods_per_line;             /*!< The number of pixel clock periods in one line (row) time. This includes visible pixels and horizontal blanking time. */
    unsigned short line_periods_per_field;             /*!< The number of complete lines (rows) in the output frame. This includes visible lines and vertical blanking lines. */
    unsigned short line_periods_vertical_blanking;     /*!< Number of vertical blanking lines. Visible lines can be calculated using this and line_periods_per_field (above) value. */
    unsigned short fine_integration_time_min;          /*!< The minimum allowed value for fine_integration_time in AEC outputs. */
    unsigned short fine_integration_time_max_margin;   /*!< fine_integration_time_max = pixel_periods_per_line - fine_integration_time_max_margin. */
    unsigned short coarse_integration_time_min;        /*!< The minimum allowed value for coarse_integration_time in AEC outputs. */
    unsigned short coarse_integration_time_max_margin; /*!< coarse_integration_time_max = line_periods_per_field - coarse_integration_time_max_margin */
} ia_aiq_exposure_sensor_descriptor;

/*!
 * AEC features.
 * Parameters for enabling/disabling AEC features. Setting ia_aiq_ae_feature_setting_tuning takes feature definitions from CPF.
 */
typedef struct {

    ia_aiq_ae_feature_setting motion_blur_control;      /*!< AEC modifies exposure time/analog gain ratio based on movement in the image. */
    ia_aiq_ae_feature_setting backlight_compensation;   /*!< AEC analyzes and modifies exposure parameters based on backlight detection algorithm. */
    ia_aiq_ae_feature_setting face_utilization;         /*!< AEC uses face coordinates in exposure calculations for next frame. */
    ia_aiq_ae_feature_setting red_eye_reduction_flash;  /*!< AEC will propose flashes before pre-flashes to reduce red eye effect. */
    ia_aiq_ae_feature_setting fill_in_flash;            /*!< AEC will propose flash in back light situations, where target is close enough. */
} ia_aiq_ae_features;


/*!
 * \brief Flash modes from the user.
 */
typedef enum
{
    ia_aiq_flash_mode_auto,
    ia_aiq_flash_mode_on,
    ia_aiq_flash_mode_off,
} ia_aiq_flash_mode;

/*!
 * \brief Flash status.
 */
typedef enum
{
    ia_aiq_flash_status_no,                /*!< No flash use. */
    ia_aiq_flash_status_torch,             /*!< Torch flash use. */
    ia_aiq_flash_status_pre,               /*!< Pre-flash use. */
    ia_aiq_flash_status_main,              /*!< Main flash use. */
    ia_aiq_flash_status_red_eye_reduction, /*!< Red Eye Reduction flash use. */
} ia_aiq_flash_status;

/*!
 * \brief Depth data type.
 */
typedef enum
{
    ia_aiq_depth_data_type_vcm,     /*!< VCM units */
    ia_aiq_depth_data_type_mm,      /*!< Distance to the object in mm */
} ia_aiq_depth_data_type;

/*!
 * \brief Flash parameters.
 * Structure can be used as input or output from AEC.
 */
typedef struct
{
    ia_aiq_flash_status status;  /*!< Flash status. */
    char power_prc;              /*!< Flash power [0,100] value range maps 0% to 100%, 0 if off. */
} ia_aiq_flash_parameters;


/*!
 * \brief Grid for weighted histograms.
 * Pixel values of certain area can be weighted differently based of specified grid.
 * Weight grid should be passed and used by the component which is calculating the histograms from the frame data. If no pixel accurate
 * histograms are calculated, the weight map should be given back to AIQ library along with the statistics so that AIQ library can
 * calculate the weighted histograms itself from the RGBS statistics.
 * This structure is output as part of AEC results but it can be replaced with custom weight map.
 */
typedef struct
{
    unsigned short width;   /*!< Width of the weight grid. */
    unsigned short height;  /*!< Height of the weight grid. */
    unsigned char *weights; /*!< Multipliers (weight) of RGB values in the grid. Values range [0, 15]. */
} ia_aiq_hist_weight_grid;

/*!
 * \brief Histogram.
 * AIQ uses internally histogram, which are calculated from RGBS statistics:
 * - "RAW" frame data i.e. RGBS statistics (corrected BLC + LSC).
 * - "Color corrected" frame data (corrected BLC + LSC + CCM + WB)
 * - "Color corrected and weighted" frame data (corrected BLC + LSC + CCM + WB + Weight Map)
 * If histograms are calculated outside AIQ from frame pixel data (more accurate), it is expected to be "Color corrected and weighted".
 * Size of histogram data arrays behind the pointers depends on value of num_bins variable in the structure.
 */
typedef struct
{
    unsigned int num_bins;            /*!< Number of histogram bins. */
    unsigned int *r;                  /*!< R histogram. */
    unsigned int *g;                  /*!< G (both Gr and Gb values) histogram. */
    unsigned int *b;                  /*!< B histogram. */
    unsigned int *rgb;                /*!< Combined RGB histogram (all pixel values of R, G and B together) TODO: Remove?. Used in percentile calculation but if GW AWB is done always, the same values are calculated. */
    unsigned int *rgb_ch;             /*!< RGB channel-independent histogram where all channels are treated as grayscale intensities and combined into one histogram. */
    unsigned int *y;                  /*!< Luminance histogram. */
    unsigned int num_r_elements;      /*!< Number of elements in the R histogram. */
    unsigned int num_g_elements;      /*!< Number of elements in the G histogram. */
    unsigned int num_b_elements;      /*!< Number of elements in the B histogram. */
    unsigned int num_rgb_elements;    /*!< Number of elements in the combined RGB histogram. */
    unsigned int num_rgb_ch_elements; /*!< Number of elements in the RGB channel-independent histogram. */
    unsigned int num_y_elements;      /*!< Number of elements in the luminance histogram. */
} ia_aiq_histogram;

/*!
 * \brief Grid block
 * As defined in the AIQ statistics specification.
 * Ranges of all parameters are [0, 255].
 */
typedef struct
{
    unsigned char avg_gr; /*!< Average Gr value in the grid. */
    unsigned char avg_r;  /*!< Average R value in the grid. */
    unsigned char avg_b;  /*!< Average B value in the grid. */
    unsigned char avg_gb; /*!< Average Gb value in the grid. */
    unsigned char sat;    /*!< Percentage of saturated pixels in the block [0, 255]. */
} rgbs_grid_block;

/*!
 * \brief R, G, B and Saturation grid block.
 * As defined in the AIQ statistics specification.
 * RGBS grid covers the full Field Of View (FOV) of the sensor.
 */
typedef struct
{
    rgbs_grid_block *blocks_ptr;  /*!< RGBS blocks. */
    unsigned short grid_width;    /*!< Grid width. */
    unsigned short grid_height;   /*!< Grid height. */
} ia_aiq_rgbs_grid; /* ia_aiq_rgbs_grid owns this */

/*!
 * \brief Grid block for 32-bit HDR.
 * As defined in the AIQ statistics specification.
 * Ranges of all parameters are [0, 255].
 */
typedef struct
{
    unsigned int avg_gr; /*!< Average Gr value in the grid. */
    unsigned int avg_r;  /*!< Average R value in the grid. */
    unsigned int avg_b;  /*!< Average B value in the grid. */
    unsigned int avg_gb; /*!< Average Gb value in the grid. */
    unsigned int sat;    /*!< Percentage of saturated pixels in the block [0, 255]. */
} hdr_rgbs_grid_block;

/*!
 * \brief R, G, B and Saturation grid block for 32-bit HDR statistics.
 * As defined in the AIQ statistics specification.
 * HDR RGBS grid covers the full Field Of View (FOV) of the sensor.
 */
typedef struct
{
    hdr_rgbs_grid_block *blocks_ptr;    /*!< HDR RGBS blocks. */
    unsigned int grid_width;            /*!< Grid width. */
    unsigned int grid_height;           /*!< Grid height. */
    unsigned int grid_data_bit_depth;   /*!< Bit depth of data in channel data. */
} ia_aiq_hdr_rgbs_grid;

/*!
 * \brief AF statistics
 * As defined in the AIQ statistics specification.
 * AF grid covers the full Field Of View (FOV) of the sensor.
 */
typedef struct
{
    unsigned short grid_width;    /*!< Number of block elements horizontally in a grid. */
    unsigned short grid_height;   /*!< Number of block elements vertically in a grid. */
    unsigned short block_width;   /*!< Block width (bq per block element). */
    unsigned short block_height;  /*!< Block height (bq per grid element). */
    int *filter_response_1;       /*!< Filter response of filter 1 (e.g. low pass, used by auto focus). */
    int *filter_response_2;       /*!< Filter response of filter 2 (e.g. high pass, used by auto focus). */
} ia_aiq_af_grid; /* ia_aiq_af_grid owns filter_response_1 and filter_response_2 */

/*!
 * \brief Depth grid statistics
 */
typedef struct
{
    ia_aiq_depth_data_type type;          /*!< Data type (VCM units/mm)*/
    ia_rectangle *grid_rect;              /*!< ROI for the grid */
    int *depth_data;                      /*!< Depth data (type of data is defined by ia_aiq_depth_data_type) */
    unsigned char *confidence;            /*!< Confidence level */
    unsigned short grid_width;            /*!< Number of grid elements horizontally */
    unsigned short grid_height;           /*!< Number of grid elements vertically */
} ia_aiq_depth_grid;

/*!
 * \brief AWB scene modes
 * Used in AWB as input to restrict White Point between certain CCT range.
 * Note that not in all cases only CCT range is used to restrict White Point but more intelligent estimation may be used.
 */
typedef enum
{
    ia_aiq_awb_operation_mode_auto,
    ia_aiq_awb_operation_mode_daylight,           /*!< Restrict CCT range to [5000, 7000]. */
    ia_aiq_awb_operation_mode_partly_overcast,    /*!< Restrict CCT range to [5500, 9000]. */
    ia_aiq_awb_operation_mode_fully_overcast,     /*!< Restrict CCT range to [6000, 7000]. */
    ia_aiq_awb_operation_mode_fluorescent,        /*!< Restrict CCT range to [2700, 5500]. */
    ia_aiq_awb_operation_mode_incandescent,       /*!< Restrict CCT range to [2700, 3100]. */
    ia_aiq_awb_operation_mode_sunset,
    ia_aiq_awb_operation_mode_video_conference,
    ia_aiq_awb_operation_mode_manual_cct_range,   /*!< Use given CCT range (see ia_aiq_awb_manual_cct_range). */
    ia_aiq_awb_operation_mode_manual_white,       /*!< Use coordinate (see ia_coordinate) relative to full FOV which should be used as white point. */
    ia_aiq_awb_operation_mode_production_test,
    ia_aiq_awb_operation_mode_candlelight,
    ia_aiq_awb_operation_mode_flash,
    ia_aiq_awb_operation_mode_snow,
    ia_aiq_awb_operation_mode_beach,
} ia_aiq_awb_operation_mode;

/*!
 * \brief Manual CCT range
 */
typedef struct
{
    unsigned int min_cct;
    unsigned int max_cct;
} ia_aiq_awb_manual_cct_range;

/*!
 * \brief Frame parameters which describe cropping and scaling (need to be filled by AIQ client for every frame)
 */
typedef struct
{
    unsigned short horizontal_crop_offset;            /*!< Read out offset horizontal. */
    unsigned short vertical_crop_offset;              /*!< Read out offset vertical. */
    unsigned short cropped_image_width;               /*!< Width of cropped area in native resolution. */
    unsigned short cropped_image_height;              /*!< Height of cropped area in native resolution. */
    unsigned char horizontal_scaling_numerator;       /*!< Horizontal scaling factor applied to the cropped image. Horizontal scaling factor = horizontal_scaling_numerator / horizontal_scaling_denominator. */
    unsigned char horizontal_scaling_denominator;     /*!< Horizontal scaling factor applied to the cropped image. Horizontal scaling factor = horizontal_scaling_numerator / horizontal_scaling_denominator. */
    unsigned char vertical_scaling_numerator;         /*!< Vertical scaling factor applied to the cropped image. Vertical scaling factor = vertical_scaling_numerator / vertical_scaling_denominator. */
    unsigned char vertical_scaling_denominator;       /*!< Vertical scaling factor applied to the cropped image. Vertical scaling factor = vertical_scaling_numerator / vertical_scaling_denominator. */
} ia_aiq_frame_params;

/*!
 * \brief DC iris control.
 * When using DC iris, there are no distinct aperture steps. Use these commands to increase or decrease aperture size.
 */
typedef enum
{
    ia_aiq_aperture_control_dc_iris_auto,  /*!< State of the iris is selected automatically. */
    ia_aiq_aperture_control_dc_iris_hold,  /*!< Iris should hold current aperture. */
    ia_aiq_aperture_control_dc_iris_open,  /*!< Iris should open. */
    ia_aiq_aperture_control_dc_iris_close, /*!< Iris should close. */
} ia_aiq_aperture_control_dc_iris_command;

/*!
 * \brief Aperture control parameters.
 * Aperture controls for both P (where iris has discrete amount of apertures) and DC (where iris has indefinite amount of possible apertures) iris.
 */
typedef struct
{
    float aperture_fn;                                       /*!< f-number of aperture (e.g. 2.8), -1.0 for N/A. Used only with P iris. */
    ia_aiq_aperture_control_dc_iris_command dc_iris_command; /*!< Used only with DC iris. */
    int code;                                                /*!< Iris module HW register value. */
} ia_aiq_aperture_control;

/*!
 * \brief Exposure data for all exposures.
 */
typedef struct
{
    unsigned int exposure_index;                        /*!< Exposure index which identifies the exposure. */
    ia_aiq_exposure_parameters* exposure;               /*!< Exposure parameters to be used in the next frame in generic format. */
    ia_aiq_exposure_sensor_parameters* sensor_exposure; /*!< Exposure parameters to be used in the next frame in sensor specific format. */
    float distance_from_convergence;                    /*!< Distance of convergence as an EV shift value. Negative is underexposure, positive is overexposure */
    bool converged;                                     /*!< Indicates that AE has converged. */
    unsigned int num_exposure_plan;                     /*!< Size of the exposure plan. Indicates how many exposure and sensor_exposure parameter structures are in the arrays above. */
    unsigned int *exposure_plan_ids;                    /*!< Exposure plan IDs. Used to identify and sync what parameters were changed in the exposure plan. */
} ia_aiq_ae_exposure_result;

/*!
 * \brief AEC results.
 */
typedef struct
{
    ia_aiq_ae_exposure_result* exposures;               /*!< Results for each exposure to be used in the next frame. */
    unsigned int num_exposures;                         /*!< The number of calculated exposures. */
    ia_aiq_hist_weight_grid* weight_grid;               /*!< Weight map to be used in the next frame histogram calculation. */
    ia_aiq_flash_parameters* flashes;                   /*!< Array of flash parameters for each flashes to be used in the next frame. */
    unsigned int num_flashes;                           /*!< Number of independent flashes. */
    unsigned int lux_level_estimate;                    /*!< Lux level estimate. */
    ia_aiq_bracket_mode multiframe;                     /*!< AEC may propose to use multiframe for optimal results. */
    ia_aiq_ae_flicker_reduction flicker_reduction_mode; /*!< Flicker reduction mode proposed by the AEC algorithm */
    ia_aiq_aperture_control *aperture_control;          /*!< Aperture control parameters. */
} ia_aiq_ae_results;

/*!
 * \brief Autofocus algorithm results
 */
typedef struct
{
    ia_aiq_af_status status;                           /*!< Focus status */
    unsigned short current_focus_distance;             /*!< Current focusing distance in mm */
    int next_lens_position;                            /*!< Next lens position */
    ia_aiq_lens_driver_action lens_driver_action;      /*!< Lens driver action*/
    bool use_af_assist;                                /*!< True if the af assist light is to be used at half press, false otherwise */
    bool final_lens_position_reached;                  /*!< Lens has reached the final lens position */
} ia_aiq_af_results;

/*!
 * \brief Results from AWB.
 */
typedef struct
{
    float accurate_r_per_g;           /*!< Accurate White Point for the image. */
    float accurate_b_per_g;           /*!< Accurate White Point for the image. */
    float final_r_per_g;              /*!< Final White Point, including color appearance modeling. */
    float final_b_per_g;              /*!< Final White Point, including color appearance modeling.*/
    unsigned int cct_estimate;        /*!< Correlated Color Temperature estimate calculated from the accurate WP. */
    float distance_from_convergence;  /*!< Range [0.0f, 1.0f]. Distance from convergence. Value 0.0f means converged. */
} ia_aiq_awb_results;

/*!
 * \brief GBCE level.
 * Allows to override GBCE level defined in the tuning.
 */
typedef enum
{
    ia_aiq_gbce_level_use_tuning = -1, /*!< Use GBCE level defined in the tuning. */
    ia_aiq_gbce_level_bypass = 0,      /*!< Use the default gamma table without stretching. This level should be used when manual AE parameters are set. */
    ia_aiq_gbce_level_gamma_stretch,   /*!< Apply histogram stretching. */
    ia_aiq_gbce_level_gamma_stretch_and_power_adaptation, /*!< Histogram stretching & gamma power adaptation. */
} ia_aiq_gbce_level;

/*!
* \brief Tone Map level.
* Allows to override Tone Map level defined in the tuning.
*/

typedef enum
{
    ia_aiq_tone_map_level_use_tuning = -1,  /*!< Use Tone Map level defined in the tuning. */
    ia_aiq_tone_map_level_bypass = 0,       /*!< Bypass TM LUT (i.e. legacy GBCE behavior) */
    ia_aiq_tone_map_level_default,          /*!< Put the gains in TM LUT (gamma LUT will have sRGB gamma only) */
    ia_aiq_tone_map_level_dynamic,          /*!< Force dynamic calculation of TM LUT */
} ia_aiq_tone_map_level;

/*!
 * \brief Results from GBCE.
 */
typedef struct {
    float* r_gamma_lut;             /*!< Gamma LUT for R channel. Range [0.0, 1.0]. */
    float* b_gamma_lut;             /*!< Gamma LUT for B channel. Range [0.0, 1.0]. */
    float* g_gamma_lut;             /*!< Gamma LUT for G channel. Range [0.0, 1.0]. */
    unsigned int gamma_lut_size;    /*!< Number of elements in each gamma LUT. */
    float* tone_map_lut;            /*!< Tone Mapping Gain LUT. Range [0.0 FLT_MAX] */
    unsigned int tone_map_lut_size; /*!< Number of elements in tone mapping LUT. */
} ia_aiq_gbce_results;

/*!
 * \brief Values used in various operations for each color channels.
 * Value range depends on algorithm output.
 */
typedef struct
{
    float gr;               /*!< A value affecting Gr color channel. */
    float r;                /*!< A value affecting R color channel. */
    float b;                /*!< A value affecting B color channel. */
    float gb;               /*!< A value affecting Gb color channel. */
} ia_aiq_color_channels;

/*!
 * \brief LUTs for each color channel.
 */
typedef struct
{
    float *gr;              /*!< LUT for Gr color channel. Range [0.0, 1.0].*/
    float *r;               /*!< LUT for R color channel. Range [0.0, 1.0]. */
    float *b;               /*!< LUT for B color channel. Range [0.0, 1.0]. */
    float *gb;              /*!< LUT for Gb color channel. Range [0.0, 1.0]. */
    unsigned int size;      /*!< Number of elements in each LUT. */
} ia_aiq_color_channels_lut;

/*!
 * \brief Shading Adaptor light source weight and its type.
 */
typedef struct {
    float weight;
    cmc_light_source source_type;
} light_source_t;

/*!
* \brief Advanced Color Correction Matrix Structure Returned by Parameter Adaptor.
*/
typedef struct {
    unsigned int sector_count;                                  /*!< Number of color matrix sectors.  */
    unsigned int *hue_of_sectors;                               /*!< Starting hues of sectors. */
    float (*advanced_color_conversion_matrices)[3][3];          /*!< Advanced CC matrices. Array of color matrices. Each color matrix optimized using a certain sector. Array size is sector_count*/
}
ia_aiq_advanced_ccm_t;



/*!
* \brief IR Weight Grid.
*/
typedef struct {
    unsigned int width;                 /*!< IR Weight grid width */
    unsigned int height;                /*!< IR Weight grid height */
    unsigned short *ir_weight_grid_R;          /*!< Interpolated IR Weight for R channel */
    unsigned short *ir_weight_grid_G;          /*!< Interpolated IR Weight for G channel */
    unsigned short *ir_weight_grid_B;          /*!< Interpolated IR Weight for B channel */
}
ia_aiq_ir_weight_t;

/*!
 * \brief Shading Adaptor results.
 */
typedef struct {
    float *channel_gr;                                /*!< Pointer to the LSC table for Gr color channel. */
    float *channel_r;                                 /*!< Pointer to the LSC table for R color channel. */
    float *channel_b;                                 /*!< Pointer to the LSC table for B color channel. */
    float *channel_gb;                                /*!< Pointer to the LSC table for Gb color channel. */
    unsigned short width;                             /*!< Width of LSC table. */
    unsigned short height;                            /*!< Height of LSC table. */
    bool lsc_update;                                  /*!< Indicates if LSC table has been modified and shall be updated in ISP. false - no change, true - new LSC. */
    light_source_t light_source[CMC_NUM_LIGHTSOURCES];/*!< Weights per light source type used in calculation of the LSC tables. */
    float scene_difficulty;                           /*!< Weighted cumulative confidence within selected patches (normalized against num_patches). */
    unsigned short num_patches;                       /*!< Number of patches that were used to make decision. */
    float covered_area;                               /*!< Statistics area covered by selected patches (%). */
    ia_aiq_frame_params frame_params;                 /*!< Frame parameters that describe cropped area size and its position under LSC grid. */
} ia_aiq_sa_results; /* ia_aiq_sa_results owns lsc_grid */

/*!
 * \brief Results from Parameter Adaptor.
 */
typedef struct {
    float color_conversion_matrix[MAX_COLOR_CONVERSION_MATRIX][MAX_COLOR_CONVERSION_MATRIX]; /*!< CC matrix. */
    ia_aiq_color_channels black_level;                /*!< Black level coefficients of each Bayer channel (absolute level). */
    ia_aiq_color_channels color_gains;                /*!< RGB gains for each color channels including given (in ia_aiq_pa_input_params) color gains and gains calculated from AWB results. */
    ia_aiq_color_channels_lut linearization;          /*!< LUTs for linearization of each color channel after black level correction. */
    float saturation_factor;                          /*!< Saturation factor to increase/decrease saturation.*/
    float brightness_level;                           /*!< Range [0.0, 1.0]. Indicates level of brightness in the image. */
    ia_aiq_advanced_ccm_t *preferred_acm;             /*!< Advanced CC matrix. */
    ia_aiq_ir_weight_t *ir_weight;                    /*!< IR Weight. */
} ia_aiq_pa_results;

/*!
 * \brief Autofocus bracketing results
 */
typedef struct
{
    unsigned short *distances_bracketing;             /*!< Ordered array of distances in mm for focus bracketing. Distances are ordered from Infinity to close up.*/
    int *lens_positions_bracketing;                   /*!< Ordered array of lens positions for focus bracketing. Order is from FAR and to NEAR end. */
} ia_aiq_af_bracket_results;

/*!
 * \brief Accelerometer Events
 *        Gravity Events
 *        Gyroscope Events
 */
typedef struct
{
    unsigned long long ts;  /*!< Time stamp in usec (microseconds) */
    float x;                /*!< Sensor Data in X direction depending on the type of the sensor */
    float y;                /*!< Sensor Data in Y direction depending on the type of the sensor */
    float z;                /*!< Sensor Data in Z direction depending on the type of the sensor */
    float sensitivity;      /*!< Sensitivity of the sensor */
    unsigned long long fs;  /*!< Frame stamp in usec (microseconds) */
} ia_aiq_sensor_data;


/* TODO: Update the structure according to the API */
/*!
 * \brief Ambient Light EventsLIGHT_AMBIENTLIGHT
 * NOTE: This should always match to libsensorhub API
 */
typedef struct
{
    unsigned long long ts;  /*!< Time stamp in usec (microseconds) */
    float data;             /*!< Ambient Light data ? */
    float sensitivity;      /*!< Sensitivity of Ambient Light sensor */
    unsigned long long fs;  /*!< Frame stamp in usec (microseconds) */
} ia_aiq_ambient_light_events;


#ifdef __cplusplus
}
#endif

#endif /* _IA_AIQ_TYPES_H_ */


