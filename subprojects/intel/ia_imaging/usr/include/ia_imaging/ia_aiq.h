/*
 * Copyright (C) 2015 - 2018 Intel Corporation.
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
 * \mainpage IA AIQ API documentation
 *
 * Browse Files and Classes tabs for details.
 *
 * \section general General info
 *
 * AIQ API has been designed to be re-entrant. Each algorithm function can be called multiple times per frame.
 * Input parameters for the algorithms define what is the output ie. running an algorithm with same input parameters
 * and same statistics produce the same output. For example one can run AE multiple times with different EV compensations
 * to get parameters for exposure bracketing.
 *
 * AIQ (Algorithms and Image Quality) library contains several algorithm which are used to modify RAW image.
 * Currently following features and algorithms are supported:
 * - \ref aec (Automatic Exposure Control)
 * - \ref awb (Automatic White Balance)
 * - \ref af (Automatic Focus)
 * - \ref sa (Shading Adaptor)
 * - \ref pa (Parameter Adaptor)
 * - \ref dsd (Discrete Scene Detection)
 * - \ref gbce (Global Brightness and Contrast Enhancement)
 *
 * AIQ also supports calculation of parameters for multiframe bracketing cases:
 * - \ref afbracket (Automatic Focus Bracket)
 * - \ref aebracket (Automatic Exposure Bracket)
 *
 * Running AIQ algorithms requires following steps:
 * - \ref init
 * - \ref stats
 * - \ref running
 * - \ref deinit
 *
 * Some AIQ algorithms require coordinates as inputs to specify a certain area in the image. Coordinates are relative to
 * the statistics thus not necessarily the whole sensor area. Coordinates are not absolute but relative. See \link ia_coordinate.h \endlink
 * for detailed description of the used coordinate system.
 * <br><hr><br>
 *
 * \section init Initialization of AIQ library
 *
 * \copybrief ia_aiq_init
 * To create an instance of AIQ library one must call function:
 * \code ia_aiq_init \endcode
 * \copydetails ia_aiq_init
 *
 * <br><hr><br>
 *
 * \section stats Setting of frame statistics
 *
 * Algorithms depend on statistics collected from the RAW image. Some or all of the statistics are
 * calculated by the ISP after RAW image capture from camera sensor. These statistics are always collected from
 * captured image data. To convert statistics from ISP specific format to AIQ format, a helper function can be used:
 * \code ia_isp_XXX_statistics_convert \endcode
 * See ia_isp documentation for details.
 *
 * \copybrief ia_aiq_statistics_set
 * To set statistics for algorithms AIQ library, one must call function:
 * \code ia_aiq_statistics_set \endcode
 * \copydetails ia_aiq_statistics_set
 *
 * Algorithms can also utilize external sensor data for making better decisions. For example external light sensor
 * can be used by AEC to determine correct cold start exposure parameters when AEC is called the first time without
 * statistics.
 *
 * \copybrief ia_aiq_sensor_events_set
 * To set external sensor data statistics for algorithms AIQ library, one must call function:
 * \code ia_aiq_sensor_events_set \endcode
 * \copydetails ia_aiq_sensor_events_set
 *
 * <br><hr><br>
 *
 * \section running Running AIQ algorithms
 *
 * Once the AIQ instance is initialized and statistics are set, algorithms can be run in any order.
 * \subsection af AF
 * \copybrief ia_aiq_af_run
 * \code ia_aiq_af_run \endcode
 * \copydetails ia_aiq_af_run
 *
 * \subsection aec AEC
 * \copybrief ia_aiq_ae_run
 * \code ia_aiq_ae_run \endcode
 * \copydetails ia_aiq_ae_run
 *
 * \subsection awb AWB
 * \copybrief ia_aiq_awb_run
 * \code ia_aiq_awb_run \endcode
 * \copydetails ia_aiq_awb_run
 *
 * \subsection sa SA
 * \copybrief ia_aiq_sa_run
 * \code ia_aiq_sa_run \endcode
 * \copydetails ia_aiq_sa_run
 *
 * \subsection pa PA
 * \copybrief ia_aiq_pa_run
 * \code ia_aiq_pa_run \endcode
 * \copydetails ia_aiq_pa_run
 *
 * \subsection dsd DSD
 * \copybrief ia_aiq_dsd_run
 * \code ia_aiq_dsd_run \endcode
 * \copydetails ia_aiq_dsd_run
 *
 * \subsection gbce GBCE
 * \copybrief ia_aiq_gbce_run
 * \code ia_aiq_gbce_run \endcode
 * \copydetails ia_aiq_gbce_run
 *
 * \subsection afbracket AF Bracket
 * \copybrief ia_aiq_af_bracket
 * \code ia_aiq_af_bracket \endcode
 * \copydetails ia_aiq_af_bracket
 *
 * \subsection aebracket AE Bracket & HDR
 * AEC supports outputting of multiple exposure results. By setting the "num_exposures" parameter >1 in ia_aiq_ae_input_params, AEC determines
 * the best exposure parameters to cover as much as possible of the sensor's dynamic range. AIQ's client can then queue the exposure parameters
 * to the sensor for consecutive frames for best speed.
 *
 * HDR support in AEC works the same way. Client requests >1 "num_exposures" but also gives AIQ the resulting statistics from all requested
 * exposures. AEC uses the given (multiple) statistics to calculate new exposure parameters.
 *
 *
 * <br><hr><br>
 *
 * \section deinit De-initialization of AIQ library
 *
 * To de-initialize and free memory AIQ library instance has allocated, one must call function:
 * \code
 * ia_aiq_deinit
 * \endcode
 *
 * After this call AIQ library instance is destroyed and can't be used.
 */

/*!
 * \file ia_aiq.h
 * \brief Definitions and declarations of Intel 3A library.
 */

#ifndef _IA_AIQ_H_
#define _IA_AIQ_H_

#include "ia_aiq_types.h"
#include "ia_types.h"
#include "ia_face.h"
#include "ia_mkn_types.h"
#include "ia_cmc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Initialize IA_AIQ and its submodules.
 * This function must be called before any other function in the library. It allocates memories for all AIQ algorithms based on input parameters
 * given by the user. AIQB (from CPFF) and NVM data are parsed and combined resulting camera module specific tuning parameters which the
 * AIQ algorithms use. Initialization returns a handle to the AIQ instance, which is given as input parameter for all the
 * algorithms. Therefore, multiple instances of AIQ library can running simultaneously. For example one instance per camera.
 *
 * \param[in]     aiqb_data         Mandatory although function will not return error, if it not given.\n
 *                                  Contains tuning parameters for AIQ algorithms.
 * \param[in]     nvm_data          Optional.\n
 *                                  NVM (Non Volatile Memory) containing sensor unit specific calibration data.
 *                                  AIC uses camera unit specific calibration data, if given.
 * \param[in]     aiqd_data         Optional.\n
 *                                  AIQ generic input data, provided by the host. Contains various AIQ related information, collected
 *                                  during run-time and stored in a host file system. AIQ will parse this data in to internal storage.
 * \param[in]     stats_max_width   Mandatory.\n
 *                                  Maximum width of RGBS and AF statistics grids from ISP. Used to calculate size of
 *                                  memory buffers for the IA_AIQ algorithms. The same maximum width will be used for all RGBS
 *                                  and AF statistics grid allocations.
 * \param[in]     stats_max_height  Mandatory.\n
 *                                  Maximum height of RGBS and AF statistics grids from ISP. Used to calculate size of
 *                                  memory buffers for the IA_AIQ algorithms. The same maximum height will be used for all RGBS
 *                                  and AF statistics grid allocations.
 * \param[in]     max_num_stats_in  Mandatory.\n
 *                                  The maximum number of input statistics for one frame. Each statistics is related to different exposure.
 *                                  Used especially for sensors that support two or more simultaneous exposures (HDR).
 * \param[in]     ia_cmc            Mandatory.\n
 *                                  Parsed camera module characterization structure. ia_cmc structure needs to be kept available by client for
 *                                  the lifetime of AIQ component.
 * \param[in,out] ia_mkn            Optional.\n
 *                                  Makernote handle which can be initialized with ia_mkn library. If debug data from AIQ is needed
 *                                  to be stored into EXIF, this parameter is needed. Algorithms will update records inside this makernote instance.
 *                                  Client writes the data into Makernote section in EXIF.
 * return                           IA_AIQ handle. Use the returned handle as input parameter for the consequent IA_AIQ calls.
 */
LIBEXPORT ia_aiq*
ia_aiq_init(const ia_binary_data *aiqb_data,
            const ia_binary_data *nvm_data,
            const ia_binary_data *aiqd_data,
            unsigned int stats_max_width,
            unsigned int stats_max_height,
            unsigned int max_num_stats_in,
            ia_cmc_t *ia_cmc,
            ia_mkn *ia_mkn);

/*!
 * \brief Set tuning to an existing AIQ instance.
 * This function can be used to switch tunings on-the-fly in a way that 3A preserves its state and offers smooth transition from one tuning to another.
 */
LIBEXPORT ia_err
ia_aiq_set_tuning(ia_aiq *ia_aiq,
                  const ia_binary_data *aiqb_data);

/*!
 * \brief De-initialize IA_AIQ and its submodules.
 * All memory allocated by AIQ algoriths are freed. AIQ handle can no longer be used.
 *
 * \param[in] ia_aiq                Mandatory.\n
 *                                  AIQ instance handle.
 */
LIBEXPORT void
ia_aiq_deinit(ia_aiq *ia_aiq);

/*!
 * \brief Manual AEC limit parameters.
 */
typedef struct
{
    int manual_exposure_time_min;             /*!< Optional. Minimum exposure time in microseconds. -1 if NA. */
    int manual_exposure_time_max;             /*!< Optional. Maximum exposure time in microseconds. -1 if NA. */
    int manual_frame_time_us_min;             /*!< Optional. Manual minimum frame length in microseconds. Defines maximum frame rate -1 if NA. */
    int manual_frame_time_us_max;             /*!< Optional. Manual maximum frame length in microseconds. Defines minimum frame rate. -1 if NA. */
    short manual_iso_min;                     /*!< Optional. Manual minimum ISO. -1 if NA. */
    short manual_iso_max;                     /*!< Optional. Manual maximum ISO. -1 if NA. */
} ia_aiq_ae_manual_limits;

/*!
 *  \brief Input parameter structure for AE algorithm.
 */
typedef struct
{
    unsigned int num_exposures;                                 /*!< Mandatory. The number of exposure outputs to have. Must be positive. One for LDR, two or more for HDR/exposure bracketing. */
    ia_aiq_frame_use frame_use;                                 /*!< Mandatory. Target frame type of the AEC calculations (Preview, Still, video etc.). */
    ia_aiq_flash_mode flash_mode;                               /*!< Mandatory. Manual flash mode. If AEC should make flash decision, set mode to ia_aiq_flash_mode_auto. */
    ia_aiq_ae_operation_mode operation_mode;                    /*!< Mandatory. AEC operation mode. */
    ia_aiq_ae_metering_mode metering_mode;                      /*!< Mandatory. AEC metering mode. */
    ia_aiq_ae_priority_mode priority_mode;                      /*!< Mandatory. AEC priority mode. */
    ia_aiq_ae_flicker_reduction flicker_reduction_mode;         /*!< Mandatory. AEC flicker reduction mode. */
    ia_aiq_exposure_sensor_descriptor *sensor_descriptor;       /*!< Mandatory although function will not return error, if not given.
                                                                     Sensor specific descriptor and limits of the used sensor mode for target frame use.
                                                                     AEC will not limit and calculate sensor specific parameters, if not given */
    ia_rectangle *exposure_window;                              /*!< Optional. Rectangle of area which AEC uses to to calculate new exposure parameters. */
    ia_coordinate *exposure_coordinate;                         /*!< Optional. Coordinate for a point in which the exposure should be prioritized.
                                                                     AEC increases weight of given point in final AEC results. */
    float ev_shift;                                             /*!< Optional. Exposure Value shift [-4,4]. */
    long *manual_exposure_time_us;                              /*!< Optional. Manual exposure time in microseconds. NULL if NA. Otherwise, array of values
                                                                     of num_exposures length. Order of exposure times corresponds to exposure_index of ae_results,
                                                                     e.g., manual_exposure_time_us[ae_results->exposures[0].exposure_index] = 33000; */
    float *manual_analog_gain;                                  /*!< Optional. Manual analog gain. NULL if NA. Otherwise, array of values of num_exposures length.
                                                                     Order of gain values corresponds to exposure_index of ae_results,
                                                                     e.g., manual_analog_gain[ae_results->exposures[0].exposure_index] = 4.0f; */
    short *manual_iso;                                          /*!< Optional. Manual ISO. Overrides manual_analog_gain. NULL if NA. Otherwise, array of values
                                                                     of num_exposures length. Order of ISO values corresponds to exposure_index of ae_results,
                                                                     e.g., manual_iso[ae_results->exposures[0].exposure_index] = 100; */
    ia_aiq_ae_features *aec_features;                           /*!< Optional. AEC features in use when calculating new exposure parameters. */
    ia_aiq_ae_manual_limits *manual_limits;                     /*!< Optional. Manual limits which override limits defined in AEC tunings. */
    float manual_aperture_fn;                                   /*!< Optional. Manual f-number of aperture (e.g. 2.8), -1.0 for N/A. Used only with P iris. */
    ia_aiq_aperture_control_dc_iris_command manual_dc_iris_command; /*!< Optional. Used only with DC iris. 0 (auto) for N/A. */
    ia_aiq_ae_exposure_distribution_priority exposure_distribution_priority; /*!< Mandatory. AEC exposure distribution priority mode. */
    float manual_convergence_time;                              /*!< Optional. Manual AEC convergence speed in seconds. -1.0 if NA. Overrides convergence speed from tunings. */
} ia_aiq_ae_input_params;

/*!
 * \brief AEC calculation based on input parameters and frame statistics.
 * AE calculates new exposure parameters to be used for the next frame based on previously given statistics and user parameters.
 *
 * \param[in] ia_aiq                Mandatory.\n
 *                                  AIQ instance handle.
 * \param[in] ae_input_params       Mandatory.\n
 *                                  Input parameters for AEC calculations.
 * \param[out] ae_results           Mandatory.\n
 *                                  Pointer's pointer where address of ISP parameters are stored.
 *                                  Results from AEC calculations. Results can be used directly as input for AIC.
 * \return                          Error code.
 */
LIBEXPORT ia_err
ia_aiq_ae_run(ia_aiq *ia_aiq,
              const ia_aiq_ae_input_params *ae_input_params,
              ia_aiq_ae_results **ae_results);

/*!
 *  \brief Input parameter structure for AF algorithm.
 */
typedef struct
{
    ia_aiq_frame_use frame_use;                                 /*!< Mandatory. Target frame type of the AF calculations (Preview, Still, video etc.). */
    int lens_position;                                          /*!< Mandatory. Current lens position. */
    unsigned long long lens_movement_start_timestamp;           /*!< Mandatory. Lens movement start timestamp in us. Timestamp is compared against statistics timestamp
                                                                     to determine if lens was moving during statistics collection. */
    ia_aiq_af_operation_mode focus_mode;                        /*!< Mandatory. Focusing mode. */
    ia_aiq_af_range focus_range;                                /*!< Mandatory. Focusing range. Only valid when focus_mode is ia_aiq_af_operation_mode_auto. */
    ia_aiq_af_metering_mode focus_metering_mode;                /*!< Mandatory. Metering mode (multispot, touch). */
    ia_aiq_flash_mode flash_mode;                               /*!< Mandatory. User setting for flash. */
    ia_rectangle *focus_rect;                                   /*!< Optional. */
    ia_aiq_manual_focus_parameters *manual_focus_parameters;    /*!< Optional. Manual focus parameters (manual lens position, manual focusing distance). Used only if
                                                                     focus mode 'ia_aiq_af_operation_mode_manual' is used. */
    bool trigger_new_search;                                    /*!< TRUE if new AF search is needed, FALSE otherwise. Host is responsible for flag cleaning. */
} ia_aiq_af_input_params;

/*!
 * \brief AF calculation based on input parameters and frame statistics.
 * AF calculates new lens position based on statistics and given input parameters.
 *
 * \param[in] ia_aiq                Mandatory.\n
 *                                  AIQ instance handle.
 * \param[in] af_input_params       Mandatory.\n
 *                                  Input parameters for AF calculations.
 * \param[out] af_results           Mandatory.\n
 *                                  Pointer's pointer where address of AF results are stored.
 *                                  Results from AF calculations.
 * \return                          Error code.
 */
LIBEXPORT ia_err
ia_aiq_af_run(ia_aiq *ia_aiq,
              const ia_aiq_af_input_params *af_input_params,
              ia_aiq_af_results **af_results);

/*!
 *  \brief Input parameter structure for AWB algorithm.
 */
typedef struct
{
    ia_aiq_frame_use frame_use;                       /*!< Mandatory. Target frame type of the AWB calculations (Preview, Still, video etc.). */
    ia_aiq_awb_operation_mode scene_mode;             /*!< Mandatory. AWB scene mode. */
    ia_aiq_awb_manual_cct_range *manual_cct_range;    /*!< Optional. Manual CCT range. Used only if AWB scene mode 'ia_aiq_awb_operation_manual_cct_range' is used. */
    ia_coordinate *manual_white_coordinate;           /*!< Optional. Manual white point coordinate relative to the full FOV of the scene. Used only if AWB scene mode 'ia_aiq_awb_operation_manual_white' is used. */
    float manual_convergence_time;                    /*!< Optional. Manual AWB convergence speed in seconds. -1.0 if NA. Overrides convergence speed from tunings. */
} ia_aiq_awb_input_params;

/*!
 * \brief AWB calculation based on input parameters and frame statistics.
 *
 * \param[in] ia_aiq                Mandatory.\n
 *                                  AIQ instance handle.
 * \param[in] awb_input_params      Mandatory.\n
 *                                  Input parameters for AWB calculations.
 * \param[out] awb_results          Mandatory.\n
 *                                  Pointer's pointer where address of AWB results are stored.
 *                                  Results from AWB calculations. Results can be used directly as input for ia_isp.
 * \return                          Error code.
 */
LIBEXPORT ia_err
ia_aiq_awb_run(ia_aiq *ia_aiq,
               const ia_aiq_awb_input_params *awb_input_params,
               ia_aiq_awb_results **awb_results);


/*!
 *  \brief Input parameter structure for GBCE algorithm.
 */
typedef struct
{
    ia_aiq_gbce_level gbce_level;           /*!< Mandatory. GBCE level. -1 to use tuning defaults.*/
    ia_aiq_tone_map_level tone_map_level;   /*!< Mandatory. Tone Map level. -1 to use tuning defaults.*/
    ia_aiq_frame_use frame_use;             /*!< Mandatory. Target frame type of the GBCE calculations (Preview, Still, video etc.). */
    float ev_shift;                         /*!< Optional. Exposure Value shift [-4,4]. */
} ia_aiq_gbce_input_params;

/*!
 * \brief GBCE calculation based on input parameters and frame statistics.
 * Computes gamma
 *
 * \param[in] ia_aiq                        Mandatory.\n
 *                                          AIQ instance handle.
 * \param[in] gbce_input_params             Mandatory.\n
 *                                          Input parameters for GBCE calculations.
 * \param[out] gbce_results                 Mandatory.\n
 *                                          Pointer's pointer where address of GBCE results are stored.
 *                                          Results contain GBCE Gamma table. Results can be used directly as input for AIC.
 * \return                                  Error code.
 */
LIBEXPORT ia_err
ia_aiq_gbce_run(ia_aiq *ia_aiq,
                const ia_aiq_gbce_input_params *gbce_input_params,
                ia_aiq_gbce_results **gbce_results);

/*!
 *  \brief Input parameter structure for DSD algorithm.
 */
typedef struct
{
    ia_aiq_af_results *af_results;     /*!< Mandatory although function will not return error, if not given.
                                            DSD will not return all scene modes, if not given. */
    ia_aiq_scene_mode scene_modes_selection;  /*!<configure which scene modes should be detected by DSD*/
} ia_aiq_dsd_input_params;

/*!
 * \brief DSD based on statistics analysis.
 * Determine scene (DSD) the given image.
 *
 * \param[in] ia_aiq                Mandatory.\n
 *                                  AIQ instance handle.
 * \param[in] dsd_input_params      Mandatory.\n
 *                                  Input parameters for DSD calculations.
 * \param[out] dsd_scene            Mandatory.\n
 *                                  Detected scene mode.
 * \return                          Error code.
 */
LIBEXPORT ia_err
ia_aiq_dsd_run(ia_aiq *ia_aiq,
               const ia_aiq_dsd_input_params *dsd_input_params,
               ia_aiq_scene_mode *dsd_scene);

/*!
 *  \brief Input parameter structure for Parameter adaptor.
 */
typedef struct
{
    ia_aiq_awb_results *awb_results;                 /*!< Mandatory. WB results which are to be used to calculate next ISP parameters (WB gains, color matrix,etc). */
    ia_aiq_exposure_parameters *exposure_params;     /*!< Mandatory. Analog gain and exposure time. */
    ia_aiq_color_channels *color_gains;              /*!< Optional. RGB gains for each color channels. These gain will be applied on top of RGB gains calculated from WB results. */
} ia_aiq_pa_input_params;

/*!
 * \brief Parameter adaptor calculations for the next frame.
 * Compute generic parameters (Color Correction Matrix and Black Level Correction),
 * which should be used to correct the next frame. Calculations are based on previously calculated AIQ algorithm results.
 * These generic results are converted to ISP specific parameters by ia_isp component.
 *
 * \param[in] ia_aiq                Mandatory.\n
 *                                  AIQ instance handle.
 * \param[in] pa_input_params       Mandatory.\n
 *                                  Input parameters for PA calculations.
 * \param[out] pa_results           Mandatory.\n
 *                                  Pointer's pointer where address of parameter adaptor results are stored.

 * \return                          Error code.
 */
LIBEXPORT ia_err
ia_aiq_pa_run(ia_aiq *ia_aiq,
               const ia_aiq_pa_input_params *pa_input_params,
               ia_aiq_pa_results **pa_results);

/*!
 *  \brief Input parameter structure for Shading Adaptor.
 */
typedef struct
{
    ia_aiq_frame_use frame_use;                      /*!< Mandatory. Target frame type of the AIC calculations (Preview, Still, video etc.). */
    ia_aiq_frame_params *sensor_frame_params;        /*!< Mandatory. Sensor frame parameters. Describe frame scaling/cropping done in sensor. */
    ia_aiq_awb_results *awb_results;                 /*!< Mandatory. WB results which are to be used to calculate next ISP parameters (WB gains, color matrix,etc). */
} ia_aiq_sa_input_params;

/*!
 * \brief Shading Adaptor calculations for the next frame.
 * Compute shading correction parameters.
 * which should be used to correct the next frame. Calculations are based on previously calculated AIQ algorithm results.
 * These generic results are converted to ISP specific parameters by ia_isp component.
 *
 * \param[in] ia_aiq                Mandatory.\n
 *                                  AIQ instance handle.
 * \param[in] sa_input_params       Mandatory.\n
 *                                  Input parameters for SA calculations.
 * \param[out] sa_results           Mandatory.\n
 *                                  Pointer's pointer where address of shading adaptor results are stored.

 * \return                          Error code.
 */
LIBEXPORT ia_err
ia_aiq_sa_run(ia_aiq *ia_aiq,
               const ia_aiq_sa_input_params *sa_input_params,
               ia_aiq_sa_results **sa_results);

/*!
 *  \brief Input parameter structure for setting the statistics.
 */
typedef struct
{
    unsigned long long frame_id;                                /*!< The frame identifier which identifies to which frame the given statistics correspond. Must be positive. */
    unsigned long long frame_timestamp;                         /*!< Mandatory although function will not return error, if not given.
                                                                     Start of frame timestamp in microseconds. This value is used in conjunction with timestamps
                                                                     provided in the AIQ algorithms function calls to calculate the convergence
                                                                     speed of AIQ algorithms.
                                                                     AEC, AWB and AF will not converge, if not given. */
    const ia_aiq_ae_results *frame_ae_parameters;               /*!< Mandatory although function will not return error, if not given.
                                                                     Exposure results from AEC which were used to capture this frame.
                                                                     AEC depends on this parameters. AEC will return cold start values if not given.*/
    const ia_aiq_af_results *frame_af_parameters;               /*!< Mandatory although function will not return error, if not given.
                                                                     Focus results from AF which were used to capture this frame.
                                                                     AEC with AF assist light and flash usage in macro functionalities depend on these parameters. */
    const ia_aiq_rgbs_grid **rgbs_grids;                        /*!< Mandatory. Almost all AIQ algorithms require RGBS grid statistics. */
    unsigned int num_rgbs_grids;                                /*!< The number of RGBS grids. */
    const ia_aiq_hdr_rgbs_grid* hdr_rgbs_grid;                  /*!< Optional. HDR statistics grid. */
    const ia_aiq_af_grid **af_grids;                            /*!< Mandatory although function will not return error, if not given.
                                                                     AF will return a NULL pointer, if not given.
                                                                     DSD will not return all scene modes, if not given. */
    unsigned int num_af_grids;                                  /*!< The number of AF grids. */
    const ia_aiq_histogram **external_histograms;               /*!< Optional. If ISP calculates histogram, if can be given. If external histogram is not given,
                                                                     AIQ internally calculates the histogram from the RGBS grid statistics and given AWB parameters. */
    unsigned int num_external_histograms;                       /*!< The number of external histograms. */
    const ia_aiq_pa_results *frame_pa_parameters;               /*!< Optional (Mandatory if external_histogram is not given).
                                                                     AWB results used in the frame from where the statistics are collected.
                                                                     GBCE will give default gamma table if external histogram or AWB results are not available. */
    const ia_face_state *faces;                                 /*!< Mandatory although function will not return error, if not given.
                                                                     Face coordinates from external face detector.
                                                                     DSD will not return all scene modes, if not given.
                                                                     AWB will not take face information into account, if not given. */
    ia_aiq_camera_orientation camera_orientation;               /*!< The orientation of the camera. Currently unused. */

    const ia_aiq_awb_results *awb_results;                      /*!< Optional. Estimated AWB results from the previous run of AWB */
    const ia_aiq_sa_results *frame_sa_parameters;               /*!< Optional. LSC results used in the frame for statistics collected. */
    const ia_aiq_depth_grid **depth_grids;                      /*!< Optional. Depth grid statistics */
    unsigned int num_depth_grids;                               /*!< Optional. Number of depth grid statistics */
} ia_aiq_statistics_input_params;

/*!
 * \brief Set input statistics and information about the captured image.
 * AIQ algorithms need various information about the conditions in which the frame and statistics were captured in order to
 * calculate new parameters.
 *
 * \param[in] ia_aiq                        Mandatory.\n
 *                                          AIQ instance handle.
 * \param[in] statistics_input_params       Mandatory.\n
 *                                          Input parameters containing statistics and information about a frame.
 * \return                                  Error code.
 */
LIBEXPORT ia_err
ia_aiq_statistics_set(ia_aiq *ia_aiq,
                      const ia_aiq_statistics_input_params *statistics_input_params);

/*!
 * \brief Focus bracketing parameters.
 */
typedef struct
{
    unsigned char focus_positions;                     /*!< Number of focus positions */
    ia_aiq_af_results af_results;                      /*!< Autofocus results */
    ia_aiq_af_bracket_mode af_bracket_mode;            /*!< Focus bracketing mode */
} ia_aiq_af_bracket_input_params;

/*!
 * \brief Calculates the list of lens positions for focus bracketing.
 *
 * \param[in]  ia_aiq                       Mandatory.\n
 *                                          AIQ instance handle.
 * \param[in]  af_bracket_input_params      Mandatory.\n
 *                                          Autofocus bracketing parameters.
 * \param[out] af_bracket_results           Mandatory.\n
 *                                          Pointer's pointer where address of focus bracketing results are stored.
 * \return                                  Error code.
 */
LIBEXPORT ia_err
ia_aiq_af_bracket(ia_aiq *ia_aiq,
                  const ia_aiq_af_bracket_input_params *af_bracket_input_params,
                  ia_aiq_af_bracket_results **af_bracket_results);

/*!
 * \param[in]  ia_aiq               Mandatory.\n
 *                                  AIQ instance handle.
 * \param[out] out_ia_aiq_data      Mandatory.\n
 *                                  Contains various AIQ related information, collected during run-time and subject to
 *                                  be stored in a host file system. Host will copy this data, if ia_aiq_data->size > 0
 *                                  and ia_aiq_data->data != NULL; AIQ is responsible to deallocate data buffer
 *                                  during ia_aiq_deinit().
 * \return                          Error code.
 */
LIBEXPORT ia_err
ia_aiq_get_aiqd_data(ia_aiq *ia_aiq,
                ia_binary_data *out_ia_aiq_data);



/*!
* \brief Data from external sensors
*/
typedef struct
{
    ia_aiq_sensor_data *accelerometer_events;                       /*!< The data holds information on the acceleration of the device in mg/sec (miligravity per second).
                                                                         Acceleration = Gravity + Linear Acceleration*/
    unsigned int num_accelerometer_events;                          /*!< Number of accelerometer events */
    ia_aiq_sensor_data *gravity_events;                             /*!< The data holds information on the gravitation of the device in mg/sec (miligravity per second) */
    unsigned int num_gravity_events;                                /*!< Number of gravity events */
    ia_aiq_sensor_data *gyroscope_events;                           /*!< The data holds information on the angular velocity of the device in rad/sec */
    unsigned int num_gyroscope_events;                              /*!< Number of gyroscope events */
    ia_aiq_ambient_light_events *ambient_light_events;              /*!< The data holds information on the ambient light */
    unsigned int num_ambient_light_events;                          /*!< Number of ambient light events */
} ia_aiq_sensor_events;

/*!
 * \brief Set event statistics.
 * Some of the AIQ algorithms benefit from sensor information which tells about the conditions in which the device is in use
 *
 * \param[in] ia_aiq                        Mandatory.\n
 *                                          AIQ instance handle.
 * \param[in] sensor_events_input           Mandatory.\n
 *                                          Sensor events input holds data from libsensorhub.
 * \return                                  Error code.
 */
LIBEXPORT ia_err
ia_aiq_sensor_events_set(ia_aiq *ia_aiq,
                        const ia_aiq_sensor_events *sensor_events_input);



/*!
 * \brief Get version.
 * Get version from version header.
 *
 * \return                                  Version string.
 */
LIBEXPORT const char* ia_aiq_get_version(void);


#ifdef __cplusplus
}
#endif

#endif /* _IA_AIQ_H_ */
