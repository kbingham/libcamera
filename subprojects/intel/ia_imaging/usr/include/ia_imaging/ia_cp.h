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

#ifndef _IA_CP_H_
#define _IA_CP_H_

/** @file ia_cp.h
 * This file declares the Intel camera computational photography API.
 */

#include "ia_types.h"
#include "ia_cp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Initialize HDR acceleration setup
 *
 * @param[out] cp_context pointer to CP instance
 * @param[in] acc_api pointer to acceleration api
 * @param[in] environment pointer to environment setup
 * @param[in] mem_environment pointer to memory environment setup
 *
 * This function initializes the acceleration setup by using input acc_api
 * and log_env.
*/
LIBEXPORT void
ia_cp_init (ia_cp_context         ** cp_context,
            const ia_acceleration * acc_api,
            const ia_env          * environment,
            const ia_mem_env      * mem_environment);


/** @brief Close HDR acceleration setup
 *
 * @param[in] cp_context pointer to CP instance
 *
 * This function frees and closes all the resources allocated for HDR
 * in its initialization call.
 */
LIBEXPORT void
ia_cp_uninit (ia_cp_context * cp_context);

/** @brief allocate and map all HDR intermediate data buffers
 *
 * @param[out] hdr pointer to HDR instance
 * @param[in] cp_context pointer to CP instance
 * @param[in] width input frame width
 * @param[in] height input frame height
 * @param[in] hdrb_data CPF binary data with tuning choices
 * @param[in] tgt target platform for HDR execution (host, ipu etc.)
 * @return error status
 *
 * This function allocates and maps all HDR intermediate buffers
 * required to do the processing. It also passes tuning choices
 * set in the CPFF for the particular platform. If hdrb_data is
 * NULL, default choices are used.
 */
LIBEXPORT ia_err
ia_cp_hdr_init (ia_cp_hdr ** hdr, ia_cp_context * cp_context, int width, int height, const ia_binary_data * hdrb_data, ia_cp_target tgt);

/** @brief release and unmap intermediate HDR data buffers
 *
 * @param[in] hdr pointer to HDR instance
 * @return error status
 *
 * This function release all resources used for intermediate
 * data storage during HDR composition.
 */
LIBEXPORT ia_err
ia_cp_hdr_uninit (ia_cp_hdr * hdr);

/** @brief compose HDR image
 *
 * @param[in] hdr pointer to HDR instance
 * @param[out] out pointer to the HDR output buffer
 * @param[out] out_pv pointer to postview output buffer
 * @param[in] in pointer to the input frame sequence
 * @param[in] in_pv pointer to the downscaled input frame sequence
 * @param[in] num_frames number of input frames
 * @param[in] cfg configuration parameters
 * @return error status
 *
 * This function composes an HDR image. It produces and output HDR frame and a downscaled version
 * of the full resolution output. Composition of the output frame is controlled via configuration
 * structure given as an input.
 */
LIBEXPORT ia_err
ia_cp_hdr_compose (ia_cp_hdr           * hdr,
                   ia_frame            * out,
                   ia_frame            * out_pv,
                   const ia_frame      * in,
                   const ia_frame      * in_pv,
                   unsigned int          num_frames,
                   const ia_cp_hdr_cfg * cfg);

/** @brief abort HDR image composition
 *
 * @param[in] hdr pointer to HDR instance
 * @return error status
 *
 * This function aborts composition of an HDR image.
 */
LIBEXPORT ia_err
ia_cp_hdr_abort (ia_cp_hdr * hdr);

/** @brief allocate and map all VHDR intermediate data buffers
 *
 * @param[out] vhdr pointer to VHDR instance
 * @param[in] cp_context pointer to CP instance
 * @param[in] width input frame width
 * @param[in] height input frame height
 * @param[in] vhdrb_data CPF binary data with tuning choices
 * @param[in] tgt target platform for VHDR execution (host, gpu etc.)
 * @return error status
 *
 * This function allocates and maps all VHDR intermediate buffers
 * required to do the processing. It also passes tuning choices
 * set in the CPF for the particular platform. If vhdrb_data is
 * NULL, default choices are used.
 */
LIBEXPORT ia_err
ia_cp_vhdr_init (ia_cp_vhdr ** vhdr, ia_cp_context * cp_context, int width, int height, const ia_binary_data * vhdrb_data, ia_cp_target tgt);

/** @brief release and unmap intermediate VHDR data buffers
 *
 * @param[in] vhdr pointer to VHDR instance
 * @return error status
 *
 * This function release all resources used for intermediate
 * data storage during VHDR composition.
 */
LIBEXPORT ia_err
ia_cp_vhdr_uninit (ia_cp_vhdr * vhdr);

/** @brief compose VHDR image
 *
 * @param[in] vhdr pointer to VHDR instance
 * @param[out] out pointer to the VHDR output buffer
 * @param[in] in pointer to the input frame sequence
 * @param[in] num_frames number of input frames
 * @param[in] cfg configuration parameters
 * @return error status
 *
 * This function composes an VHDR image. It produces and output VHDR frame.
 * Composition of the output frame is controlled via configuration
 * structure given as an input.
 */
LIBEXPORT ia_err
ia_cp_vhdr_compose (ia_cp_vhdr           * vhdr,
                    ia_frame             * out,
                    const ia_frame       * in,
                    unsigned int           num_frames,
                    const ia_cp_vhdr_cfg * cfg);

/** @brief abort VHDR image composition
 *
 * @param[in] vhdr pointer to VHDR instance
 * @return error status
 *
 * This function aborts composition of an VHDR image.
 */
LIBEXPORT ia_err
ia_cp_vhdr_abort (ia_cp_vhdr * vhdr);

/** @brief allocate and map all VHDR intermediate data buffers
 *
 * @param[out] vhdr address of pointer to VHDR instance
 * @param[in] cp_context pointer to CP instance
 * @param[in] width input frame width
 * @param[in] height input frame height
 * @param[in] vhdrb_data CPF binary data with tuning choices
 * @param[in] tgt target platform for VHDR execution (host, gpu etc.)
 * @return error status
 *
 * This function allocates and maps all VHDR intermediate buffers
 * required to do the processing. It also passes tuning choices
 * set in the CPF for the particular platform. If vhdrb_data is
 * NULL, default choices are used.
 */
LIBEXPORT ia_err
ia_cp_vhdr_v2_init (ia_cp_vhdr_v2 **vhdr, ia_cp_context *cp_context, int width, int height, const ia_binary_data *vhdrb_data, ia_cp_target tgt);

/** @brief release and unmap intermediate VHDR data buffers
 *
 * @param[in] vhdr pointer to VHDR instance
 * @return error status
 *
 * This function release all resources used for intermediate
 * data storage during VHDR composition.
 */
LIBEXPORT ia_err
ia_cp_vhdr_v2_uninit (ia_cp_vhdr_v2 *vhdr);

/** @brief compose VHDR image
 *
 * @param[in] vhdr pointer to VHDR instance
 * @param[out] out pointer to the VHDR output buffer
 * @param[in] in pointer to the input frame sequence
 * @param[in] cfg configuration parameters
 * @return error status
 *
 * This function composes an VHDR image. It produces and output VHDR frame.
 * Composition of the output frame is controlled via configuration
 * structure given as an input.
 */
LIBEXPORT ia_err
ia_cp_vhdr_v2_compose (ia_cp_vhdr_v2        *vhdr,
                       ia_frame             *out,
                       const ia_frame       *in,
                       const ia_cp_vhdr_cfg *cfg);

/** @brief initialize ULL parameters
 *
 * @param[out] ull pointer to ULL instance
 * @param[in] cp_context pointer to CP instance
 * @param[in] width input frame width
 * @param[in] height input frame height
 * @param[in] ullb_data CPF binary data with tuning choices
 * @param[in] tgt target platform for ULL execution (host, ipu etc.)
 * @return error status
 *
 * This function initializes internals for ULL processing. It also
 * passes tuning choices set in the CPFF for the particular platform.
 * If ullb_data is NULL, default choices are used.
 */
LIBEXPORT ia_err
ia_cp_ull_init (ia_cp_ull ** ull, ia_cp_context * cp_context, int width, int height, const ia_binary_data * ullb_data, ia_cp_target tgt);

/** @brief deinitialize ULL internals
 *
 * @param[in] ull pointer to ULL instance
 * @return error status
 *
 * This function release all resources used during ULL composition.
 */
LIBEXPORT ia_err
ia_cp_ull_uninit (ia_cp_ull * ull);

/** @brief compose ULL image
 *
 * @param[in] ull pointer to ULL instance
 * @param[out] out pointer to the ULL output buffer
 * @param[out] out_pv pointer to postview output buffer
 * @param[in] in pointer to the input frame sequence
 * @param[in] in_pv pointer to the downscaled input frame sequence
 * @param[in] num_frames number of input frames
 * @param[in] cfg configuration parameters
 * @return error status
 *
 * This function composes a denoised image from a set of input images captured in exteme low-light
 * conditions. It produces an output frame and a downscaled version of the full resolution output.
 * Composition of the output frame is controlled via configuration structure given as an input.
 */
LIBEXPORT ia_err
ia_cp_ull_compose (ia_cp_ull           * ull,
                   ia_frame            * out,
                   ia_frame            * out_pv,
                   const ia_frame      * in,
                   const ia_frame      * in_pv,
                   unsigned int          num_frames,
                   const ia_cp_ull_cfg * cfg);

/** @brief abort ULL image composition
 *
 * @return error status
 *
 * This function aborts composition of a ULL image.
 */
LIBEXPORT ia_err
ia_cp_ull_abort (void);

/** @brief Estimate global motion between two frames provided in a form of gaussian pyramids
 *
 * @param[out] result outcome of the estimation process
 * @param[in] target_pyr array of target frame pyramid levels
 * @param[in] source_pyr array of source frame pyramid levels
 * @param[in] cfg configuration parameters
 * @return error status
 *
 * This function estimates global motion between the source frame and the target (base) frame given in a form of
 * gaussian pyramids. Results are produced in a form of a 3x3 global transformation matrix sufficient to cover the
 * most complex use case of projective transformation. Estimation also advocates fallback in case the global motion
 * was sufficiently large.
 *
 */
LIBEXPORT ia_err
ia_cp_global_me_multires (ia_cp_me_result * result, const ia_frame target_pyr[],
                          const ia_frame source_pyr[], const ia_cp_me_cfg * cfg);

/** @brief Estimate global motion between two frames
 *
 * @param[out] result outcome of the estimation process
 * @param[in] target pointer to the target buffer
 * @param[in] source pointer to the source buffer
 * @param[in] cfg configuration parameters
 * @return error status
 *
 * This function estimates global motion between the source frame and the target (base) frame. Results are
 * produced in a form of a 3x3 global transformation matrix sufficient to cover the most complex use case
 * of projective transformation. Estimation also advocates fallback in case the global motion was
 * sufficiently large.
 *
 */
LIBEXPORT ia_err
ia_cp_global_me (ia_cp_me_result * result, const ia_frame * target, const ia_frame * source,
                 const ia_cp_me_cfg * cfg);

/** @brief Compensate frame motion based on the transformation matrix
 *
 * @param[out] target pointer to the target buffer
 * @param[in] source pointer to the source buffer
 * @param[in] result outcome of the estimation process
 * @return error status
 *
 * This function compensates global motion based on the provided motion estimation results.
 *
 */
LIBEXPORT ia_err
ia_cp_global_mc (ia_frame * target, ia_frame * source, ia_cp_me_result * result);


/** @brief Zoom provided frame with specified zoom factor
 *
 * @param[inout] in_out frame
 * @param[in] zoom_factor
 * @return error status
 *
 * This function performs zooming of input frame with specified zoom factor.
 *
 */
LIBEXPORT ia_err
ia_cp_zoom_frame(ia_frame * in_out, int zoom_factor);

/** @brief Load extension binaries
 *
 * @param[in] cp_context pointer to CP instance
 * @return error status
 *
 * This function loads CP binaries as an extension to the preview pipe.
 */
LIBEXPORT ia_err
ia_cp_load_extensions (ia_cp_context * cp_context);

/** @brief Unload extension binaries
 *
 * @param[in] cp_context pointer to CP instance
 * @return error status
 *
 * This function unloads CP binaries which are previously loaded as an extension
 * to the preview pipe.
 */
LIBEXPORT ia_err
ia_cp_unload_extensions (ia_cp_context * cp_context);

/**
 * @brief   Computes standard deviation for the patches in the provided chart
 *
 * @param[in]   chart             Chart descriptor
 * @param[in]   mean_min          Minimal valid patch mean
 * @param[in]   mean_max          Maximal valid patch mean
 * @param[out]  deviation         Average deviation for valid patches
 *
 * @return                        Flag indicating success or failure
 *
 * Function computes standard deviation for the patches in the provided chart. Chart data type contains
 * a frame and location of all patches of interest. Expected are 8-bit frames, computation is done only
 * for the first plane in the frame. Resulting deviation is scaled into the [0, 1) range.
 */
LIBEXPORT ia_err
ia_cp_chart_compute_deviation (ia_cp_chart chart, int mean_min, int mean_max, float *deviation);

/**
 * @brief   Performs linear regression fitting for the given dataset
 *
 * @param[in]   psrc_x            Pointer to dataset X values
 * @param[in]   psrc_y            Pointer to dataset Y values
 * @param[in]   len               Length of the dataset
 * @param[out]  slope             Slope of the resulting linear curve
 * @param[out]  offset            Offset of the resulting linear curve
 *
 * @return                        Flag indicating success or failure
 *
 * Function performs linear curve fitting for the given dataset via means of linear regression. Resulting
 * slope and offset describe the linear fit as y = slope * x + offset.
 */
LIBEXPORT ia_err
ia_cp_linear_regression (const float *psrc_x, const float *psrc_y, int len, float *slope, float *offset);

#ifdef __cplusplus
}
#endif

#endif /* _IA_CP_H */
