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
 * \file ia_mkn_encoder.h
 * \brief Definitions of functions to control and add records to Maker Note.
*/

#ifndef IA_MKN_ENCODER_H_
#define IA_MKN_ENCODER_H_

#include "ia_types.h"
#include "ia_mkn_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Creates Make Note system handle.
 * Allocates and initializes makernote handle. Handle must be given as input parameter to all consequent ia_mkn calls.
 *
 * \param[in] mkn_config_bits       Mandatory.\n
 *                                  Configuration flag bits.
 * \param[in] mkn_section_1_size    Mandatory.\n
 *                                  Size of Section 1 data buffer.
 * \param[in] mkn_section_2_size    Mandatory.\n
 *                                  Size of Section 2 data buffer.
 * \return                          Pointer to the makernote handle.
 */
LIBEXPORT ia_mkn*
ia_mkn_init(ia_mkn_config_bits mkn_config_bits,
            size_t mkn_section_1_size,
            size_t mkn_section_2_size);

/*!
 * \brief Deletes Make Note system handle.
 *
 * \param[in] mkn                Mandatory. \n
 *                               Pointer to makernote handle.
 * \return                       Error code.
 */
LIBEXPORT void
ia_mkn_uninit(ia_mkn *mkn);

/*!
 * \brief Reset Make Note system to default state.
 *
 * \param[in] mkn                Mandatory. \n
 *                               Pointer to makernote handle obtained from ia_mkn_init function call.
 * \return                       Error code.
 */
LIBEXPORT ia_err
ia_mkn_reset(ia_mkn *mkn);

/*!
 * \brief Adds or updates a data record in the makernote.
 *
 * \param[in] mkn                Mandatory. \n
 *                               Pointer to makernote handle obtained from ia_mkn_init function call.
 * \param[in] data_format_id     Mandatory.\n
 *                               Record data format ID.
 * \param[in] data_name_id       Mandatory.\n
 *                               Record name ID.
 * \param[in] data               Mandatory.\n
 *                               Pointer to record data to be copied into the makernote. Size of data to be copied is calculated
 *                               from on DFID and number of elements.
 * \param[in] num_elements       Mandatory.\n
 *                               Number of elements to store.
 * \param[in] key                Mandatory.\n
 *                               Packing key (16 bytes). NULL means 'no packing'.
 * \return                       Error code.
*/
LIBEXPORT ia_err
ia_mkn_add_record(ia_mkn *mkn,
                  ia_mkn_dfid mkn_data_format_id,
                  ia_mkn_dnid mkn_data_name_id,
                  const void *data,
                  unsigned int num_elements,
                  const char *key);

/*!
 * \brief Deletes a data record from the makernote.
 *
 * \param[in] mkn                Mandatory. \n
 *                               Pointer to makernote handle obtained from ia_mkn_init function call.
 * \param[in] data_format_id     Mandatory.\n
 *                               Record data format ID.
 * \param[in] data_name_id       Mandatory.\n
 *                               Record name ID.
 * \return                       Error code.
*/
LIBEXPORT ia_err
ia_mkn_delete_record(ia_mkn *mkn,
                     ia_mkn_dfid mkn_data_format_id,
                     ia_mkn_dnid mkn_data_name_id);

/*!
 * \brief Prepares makernote so that it can be included into the EXIF.
 * Based on data target: Section 1 can be used by client for example for EXIF or Section 2 where all (Section 1 & Section 2) records will be included.
 * calculates checksum, updates total size of makernote data, compresses and pack makernote data.
 *
 * \param[in] mkn                Mandatory. \n
 *                               Pointer to makernote handle obtained from ia_mkn_init function call.
 * \param[in] data_target        Target of the makernote as defined in enum ia_mkn_trg.
 * \return                       Binary data structure with pointer and size of data..
 */
LIBEXPORT ia_binary_data
ia_mkn_prepare(ia_mkn *mkn,
               ia_mkn_trg data_target);

/*!
 * \brief Enable/Disable makernote data collecting.
 *
 * \param[in] mkn                Mandatory. \n
 *                               Pointer to makernote handle obtained from ia_mkn_init function call.
 * \param enable_data_collection Mandatory.\n
 *                               Enable/disable data collection.
 * \return                       Error code.
*/
LIBEXPORT ia_err
ia_mkn_enable(ia_mkn *mkn,
              bool enable_data_collection);

/*!
 * \brief Merge two makernotes.
 * Copies all records from source makernote to the target makernote. Existing same records in the target are overwritten by source record.
 * Both makernotes must be created with the same makernote library ie. have the same internal structure.
 * After merging makernotes, ia_mkn_prepare() function must be called before using the merged makernote.
 *
 * \param[in,out] mkn_trg_data   Target makernote. Source makernote will be merged into this.
 * \param[in]     mkn_src_data   Source makernote.
 * \return                       Error code.
 */
LIBEXPORT ia_err
ia_mkn_merge(ia_mkn *mkn_trg,
             const ia_mkn *mkn_src);

/*!
 * \brief Converts makernote (MKNT) binary data to full MKN data.
 *  Allocates full MKN data and copies the content of (MKNT) binary data to MKN.
 *
 * \param[in]     mknt_src_data  Pointer to the makernote (MKNT) binary data.
 * \return                       Pointer to the makernote handle.
 */
LIBEXPORT ia_mkn*
ia_mknt_to_mkn(const ia_binary_data *mknt_src_data);

#ifdef __cplusplus
}
#endif

#endif /* IA_MKN_ENCODER_H_ */
