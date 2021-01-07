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
 * \mainpage IA LTM API documentation
 *
 * Browse Files and Classes tabs for details.
 *
 * \section general General info
 *
 *  \section init Initialization of LTM library
 *
 * \copybrief ia_ltm_init
 * To create an instance of LTM library one must call function:
 * \code ia_ltm_init \endcode
 * \copydetails ia_ltm_init
 *
 * <br><hr><br>
 */
/*!
 * \file ia_ltm.h
 * \brief Definitions and declarations of Intel LTM library.
 */


#ifndef _IA_LTM_H_
#define _IA_LTM_H_

#include "ia_ltm_types.h"
#include "ia_types.h"
#include "ia_aiq_types.h"
#include "ia_mkn_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Initialize IA_LTM.
 * This function must be called before any other function in the library. It allocates memories for all LTM algorithms based on input parameters
 * given by the user. Tuning parameters are parsed from AIQB and saved for LTM algorithms to use. Initialization returns a handle to the LTM instance,
 * which is given as input parameter for other functions to access object data. Therefore, multiple instances of LTM library can running simultaneously.
 * For example one instance per camera.
 *
 * \param[in]     aiqb_data         Mandatory although function will not return error, if it not given.\n
 *                                  AIQB Block from CPFF. Contains tuning parameters for AIQ, ISP and LTM algorithms.
 * \param[in,out] ia_mkn            Optional.\n
 *                                  Makernote handle which can be initialized with ia_mkn library. If debug data from LTM is needed
 *                                  to be stored into EXIF, this parameter is needed. Algorithms will update records inside this makernote instance.
 *                                  Client writes the data into Makernote section in EXIF.
 * return                           IA_LTM handle. Use the returned handle as input parameter for the consequent IA_LTM calls.
 */
LIBEXPORT ia_ltm*
ia_ltm_init(const ia_binary_data *aiqb_data,
            ia_mkn *ia_mkn);

/*!
 * \brief De-initialize IA_LTM.
 * All memory allocated by LTM algorithms is freed. LTM handle can no longer be used.
 *
 * \param[in] ia_ltm                Mandatory.\n
 *                                  LTM instance handle.
 */
LIBEXPORT void
ia_ltm_deinit(ia_ltm *ia_ltm);

/*!
 * \brief Input parameter structure for LTM/DRC algorithm.
 * Although all the input statistics and image are optional, one of them is always needed.
 * While certain LTM algorithms (selectable from tunings) can utilize different input, all algorithms can operate on ia_ltm_input_image data.
 * Notice that input image or statistics may or may not contain WB gains and CCM applied.
 */
typedef struct ia_ltm_input_params
{
   ia_ltm_level ltm_level;                 /*!< Mandatory. LTM level. -1 to use tuning defaults.*/
   ia_aiq_frame_use frame_use;             /*!< Mandatory. Target frame type of the LTM calculations (Preview, Still, video etc.). */
   float ev_shift;                         /*!< Optional. Exposure Value shift [-4,4]. */
   char ltm_strength_manual;               /*!< Optional. user defined manual control for ltm strength, will be casted into unsigned char, [0, 200], default is 100 and means no manual effect*/
   ia_aiq_ae_results *ae_results;          /*!< Optional. AEC output will be used by LTM.*/
   int16_t frame_width;                    /*!< Mandatory. Width of the frame where the results will be applied. */
   int16_t frame_height;                   /*!< Mandatory. Height of the frame where the results will be applied. */
   ia_aiq_rgbs_grid *rgbs_grid_ptr;        /*!< Optional. RGBS statistics. LTM may use this small grid instead of given larger grids to reduce PnP (available in IPU4 and onwards). */
   ia_aiq_hdr_rgbs_grid *hdr_rgbs_grid_ptr;/*!< Optional. HDR RGBS statistics. LTM may use this small grid instead of given larger grids to reduce PnP (available in IPU4 and onwards). */
   ia_ltm_input_image *input_image_ptr;    /*!< Optional. Image data of any resolution based on IQ requirements for particular use case from which LTM calculates local tone maps (HW generated image available in IPU5 and onwards). */
} ia_ltm_input_params;

/*!
 * \brief LTM calculation based on input parameters and frame statistics.
  *
 * \param[in] ia_ltm                        Mandatory.\n
 *                                          LTM instance handle.
 * \param[in] ltm_input_params              Mandatory.\n
 *                                          Input parameters for LTM calculations.
 * \param[out] ltm_results                  Mandatory.\n
 *                                          Pointer's pointer where address of LTM results are stored.
 * \return                                  Error code.
 */
LIBEXPORT ia_err
ia_ltm_run(ia_ltm *ia_ltm,
        const ia_ltm_input_params *ltm_input_params,
        ia_ltm_results **ltm_results,
        ia_ltm_drc_params **ltm_results_drc);

/*!
 * \brief Get version.
 * Get version from version header.
 *
 * \return                                  Version string.
 */
LIBEXPORT const char* ia_ltm_get_version(void);


#ifdef __cplusplus
}
#endif

#endif /* _IA_LTM_H_ */
