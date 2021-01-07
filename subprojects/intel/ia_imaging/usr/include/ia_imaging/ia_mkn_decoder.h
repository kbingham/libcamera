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
 * \file ia_mkn_decoder.h
 * \brief Definitions of functions to decode records from Maker Note.
*/

#ifndef IA_MKN_DECODER_H_
#define IA_MKN_DECODER_H_

#include "ia_types.h"
#include "ia_mkn_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Checks if makernote contents are valid.
 *
 * \param[in] a_mknt_data_ptr       Mandatory.\n
 *                                  Pointer to the makernote (MKNT) binary data.
 * \param[in] a_tag                 Mandatory.\n
 *                                  Tag which should match tag in the given data. Can be 0, if tag doesn't matter.
 * \return                          True if makernote data is valid.
 */
LIBEXPORT bool
ia_mkn_is_valid(const ia_binary_data *a_mknt_data_ptr,
                const unsigned int a_tag);

/*!
 * \brief Changes endianness of given makernote buffer.
 * Modifies the makernote buffer by changing endianness of makernote header and records.
 *
 * \param[in, out] mknt_data        Mandatory.\n
 *                                  Pointer to the makernote (MKNT) binary data which will be changed
 *                                  to different endianness.
 * \return                          Error code.
 */
LIBEXPORT ia_err
ia_mkn_change_endianness(ia_binary_data *mknt_data);

/*!
 * \brief Prints all records contents.
 * Prints all record headers and record contents into the stdout in the same format as defined by the DFID. If a buffer
 * containing makernote header file is given as input, the DNID is also printed out as the first value on each row.
 * Note. Makernote data CRC validity is not checked when printing records. Prior to calling this function,
 * call ia_mkn_is_valid() to validate integrity of makernote data.
 *
 * \param[in] mknt_data             Mandatory.\n
 *                                  Pointer to the makernote (MKNT) binary data.
 * \param[in] makernote_header_file Optional.\n
 *                                  Buffer where makernote header file has been read. Can be NULL.
 * \param[in] mkn_dnid_struct_name  Optional.\n
 *                                  C string of name of structure containing DNIDs in the given header file.
 * \param[in] dnid                  Mandatory.\n
 *                                  Record's DNID to print. If 0, all records will be printed out.
 * \param[in] binary                Mandatory.\n
 *                                  Flag indicating if data is printed out as binary data into stdout.
 * \param[in] key                   Mandatory.\n
 *                                  Packing key (16 bytes).
 * \return                          Error code.
 */
LIBEXPORT ia_err
ia_mkn_print_record(const ia_binary_data *mknt_data,
                    const char *makernote_header_file,
                    const char *mkn_dnid_struct_name,
                    ia_mkn_dnid dnid,
                    bool binary,
                    const char *key);

/*!
 * \brief Copies record data from the makernote to given buffer.
 * Checks if a given record exists in the makernote and copies the data from the makernote buffer into the record data buffer.
 * The amount to copy depends on size value given as input in the record header structure. If size is 0,
 * only the record header is updated with correct data size and no data is copied. Thus this function can be called twice:
 * First to get the record size and second time (after allocating a buffer for the output) to get the record data. When querying
 * for record, DFID and DNID must match the record's DFID and DNID.
 * Note. Makernote data CRC validity is not checked when getting records. Prior to calling this function,
 * call ia_mkn_is_valid() to validate integrity of makernote data.
 *
 * \param[in] mknt_data             Mandatory.\n
 *                                  Pointer to the makernote (MKNT) binary data.
 * \param[in] key                   Mandatory.\n
 *                                  Packing key (16 bytes).
 * \param[in,out] mkn_record_header Mandatory.\n
 *                                  Record header with size set to 0 or wanted data size from record. DFID and DNID
 *                                  must be set correctly to get record data.
 * \param[out]    record_data       Mandatory if record header size is not 0.\n
 *                                  Large enough buffer to hold whole record data.
 * \return                          Error code.
 */
LIBEXPORT ia_err
ia_mkn_get_record(const ia_binary_data *mknt_data,
                  const char* key,
                  ia_mkn_record_header *mkn_record_header,
                  void *record_data);

/*!
 * \brief Copies record data from the makernote to given buffer.
 * Parses through the makernote file and copies record headers of the first num_mkn_records to the given memory array.
 * Client should make sure enough memory is allocated for num_mkn_records in the given array.
 * If mkn_record_headers is NULL, this function will return the number of records. Thus
 * first call can be used to query how many records there are and second call to get the actual record headers.
 *
 * \param[in] mknt_data              Mandatory.\n
 *                                   Pointer to the makernote (MKNT) binary data.
 * \param[in,out] num_mkn_records    Mandatory.\n
 *                                   Number of makernote records the function is allowed to parse and store to the mkn_record_headers.
 * \param[in,out] mkn_record_headers Mandatory.\n
 *                                   Client allocated memory for storing the array of record headers parsed by this function.
 * \return                           Error code.
 */
LIBEXPORT ia_err
ia_mkn_get_record_headers(const ia_binary_data *mknt_data,
                          int *num_mkn_records,
                          ia_mkn_record_header *mkn_record_headers);

#ifdef __cplusplus
}
#endif
#endif /* IA_MKN_DECODER_H_ */
