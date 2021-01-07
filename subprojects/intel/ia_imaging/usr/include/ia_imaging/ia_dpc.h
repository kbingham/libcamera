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
 * \mainpage IA DPC API documentation
 *
 * Browse Files and Classes tabs for details.
 *
 */
/*!
 * \file ia_dpc.h
 * \brief Definitions and declarations of IA SD-DPC library.
 */
#ifndef _IA_DPC_H_
#define _IA_DPC_H_

#include "ia_aiq_types.h"
#include "ia_types.h"
#include "ia_mkn_types.h"
#include "ia_cmc_types.h"

#ifdef __cplusplus
extern "C" {
#endif
#define IA_DPC_VERSION "1.0.0.0"
typedef struct ia_dpc_t ia_dpc;

/*!
 * \brief SD-DPC runtime input.
 */
typedef struct {
    short *frame_data;                 /*!< fixed point image */
    int frame_width;                   /*!< current frame width (might be cropped) */
    int frame_height;                  /*!< current frame height (might be cropped) */
} sd_dpc_input;

/*!
 * \brief SD-DPC per-run output.
 */
typedef struct {
    short *defect_lut;          /*!< defects LUT stores the column indices (one-based) of the approved defects per row. size frame_height x max_defects_per_line */
    int dpc_lut_width;        /*!< the width of the DP LUT, equal to maximum number of defects per line */
    int dpc_lut_height;       /*!< the height of the DP LUT, equal to sensor height */
    bool dpc_lut_changed;     /*!< true if the new defects have been added */
} sd_dpc_output;

/*!
 * \brief Initialize IA_DPC.The function parses the given AIQB data and determines if the SD-DPC should be used.
 *        If yes the object is allocated and initialized.
 *        If not, null is returned
 *
 * \param[in]     aiqb_data              Mandatory.\n
 *                                       CPFF that contains tuning parameters for camera, ISP and AIQ algorithms.
 *                                       If NULL is given the function will return NULL.
 * \param[in]     ia_cmc                 Mandatory.\n
 *                                       Parsed camera module characterization structure. Internal copy of the structure will be taken.
 *                                       If NULL is given the function will return NULL.
 * \param[in]     ia_dpcd_data           Mandatory.\n
 *                                       loaded DPCD data. Should not be NULL, but have size = 0 instead
 * \param[in]     max_defects_per_line   Mandatory.\n
 *                                       maximum number of defects per line support by the ISP.
 * \param[in]     stripe_number_of_lines Mandatory.\n
 *                                       The maximum stripe height.
 * return                                IA_DPC handle. Use the returned handle as input parameter for the consequent IA_DPC calls.
 *                                       Returns NULL is SD-DPC should be bypassed.
 */
LIBEXPORT ia_dpc*
ia_dpc_init(const ia_binary_data *aiqb_data,
            const ia_cmc_t *ia_cmc,
            const ia_binary_data *ia_dpcd_data,
            unsigned int max_defects_per_line,
            unsigned int stripe_number_of_lines);

/*!
 * \brief De-initialize IA_DPC.
 * All memory allocated by DPC algoriths are freed. DPC handle can no longer be used.
 *
 * \param[in] ia_dpc                Mandatory.\n
 *                                  DPC instance handle.
 */
LIBEXPORT void
ia_dpc_deinit(ia_dpc *ia_dpc);
/*!
 * \brief SD-DPC execution based on input parameters and stripe.
 *
 *  \param [in]      ia_dpc         Mandatory.\n
 *                                  SD-DPC state, updated with the intermediate results
 *  \param [in,out]  stripe_ptr     Mandatory.\n
 *                                  Stripe data that include the RAW pixels and the exisitng/ missing margins information.
 *  \param [out]     dpc_output     Mandatory.\n
 *                                  Output LUT of the verified defect pixels. The new defect pixels can be only in the given stripe region.
 *                                  Results can be used directly as input for AIC.
 *  \return                         Error code.
 */

LIBEXPORT ia_err
ia_dpc_run(ia_dpc *ia_dpc,
          const sd_dpc_input *stripe_ptr,
          const ia_aiq_frame_params *aiq_frame_params_ptr,
          const ia_aiq_ae_exposure_result *ae_exposure_result_ptr,
          sd_dpc_output *dpc_output);

/*!
 * \param[in]  ia_aiq               Mandatory.\n
 *                                  DPC instance handle.
 * \param[out] out_ia_dpcd_data     Mandatory.\n
 *                                  Contains various DPC related information, collected during run-time and subject to
 *                                  be stored in a host file system. Host will copy this data, if ia_dpcd_data->size > 0
 *                                  and ia_dpcd_data->data != NULL; SD-DPC is responsible to deallocate data buffer
 *                                  during ia_dpc_deinit().
 * \return                          Error code.
 */
LIBEXPORT ia_err
ia_dpc_get_dpcd_data(
        ia_dpc *ia_dpc,
        ia_binary_data *out_ia_dpcd_data);

#ifdef __cplusplus
}
#endif
#endif /* _IA_DPC_H_ */
