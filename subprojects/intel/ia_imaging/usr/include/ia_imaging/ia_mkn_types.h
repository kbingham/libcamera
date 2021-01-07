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
 * \file ia_mkn_types.h
 * \brief Enumerations, structures and definitions used in the Maker Note System.
*/

#ifndef _IA_MKN_TYPES_H_
#define _IA_MKN_TYPES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief combines 4 chars into one unsigned long integer.
 */
#define IA_MKN_CHTOUL(a,b,c,d) \
    ( (uint32_t)(a) | ((uint32_t)(b)<<8) | ((uint32_t)(c)<<16) | ((uint32_t)(d)<<24) )

/*!
 * \brief Makernote tag. 4 first bytes of the makernote data.
 */
#define IA_MKN_TAG IA_MKN_CHTOUL('M','K','N','T')

/*!
 * \brief Definition of ia_mkn handle.
 * Definition to clarify when ia_mkn handle is expected to be used.
 */
typedef struct ia_mkn_t ia_mkn;

/*!
 *  Start of MKN System enumeration set.
 * \brief Enumerations set for Maker Note System.
 * This part contatins two enumerations:
 * - 'ia_mkn_dfid', describes data type of the 'Data' field in the MKN record;
 * - 'ia_mkn_dnid',describes a name (or functionality) of data in the 'Data' field.
 * To modify this file a following procedure is used:
 * a) Add new enum members of ia_mkn_dfid or ia_mkn_dnid
 * b) Update MKN_ENUMS_DATA_REVISION accordingly.
 */

/*!
 *  Revision of MKN System enumeration set, format 0xYYMMDDVV, where:
 * - YY: year,
 * - MM: month,
 * - DD: day,
 * - VV: version ('01','02' etc.)
 */
#define IA_MKN_ENUMS_DATA_REVISION 0x14012702

/*!
 * \brief Data Format ID (DFID) enumeration describes data type of the 'Data' field in the MKN record.
 */
typedef enum
{
    ia_mkn_dfid_dummy,                     /*!< Used for dummy records (no actual data). */

    /*! Standard integer data types */
    ia_mkn_dfid_signed_char,               /*!< 8-bit, int. values, range: -128 to 127 */
    ia_mkn_dfid_unsigned_char,             /*!< 8-bit, int. values, range: 0 to 255 */
    ia_mkn_dfid_signed_short,              /*!< 16-bit, int. values, range: -32768 to 32767 */
    ia_mkn_dfid_unsigned_short,            /*!< 16-bit, int. values, range: 0 to 65535 */
    ia_mkn_dfid_signed_int,                /*!< 32-bit, int. values, range: -2147483648 to 2147483647 */
    ia_mkn_dfid_unsigned_int,              /*!< 32-bit, int. values, range: 0 to 4294967295 */
    ia_mkn_dfid_signed_long_long,          /*!< 64-bit, int. values, range: -9223372036854775808 to 9223372036854775807 */
    ia_mkn_dfid_unsigned_long_long,        /*!< 64-bit, int. values, range: 0 to 18446744073709551615 */
    ia_mkn_dfid_string,                    /*!< 8-bit chars which are converted to ASCII. */

    /*! Floating point numbers. */
    ia_mkn_dfid_float = 14,                /*!< IEEE-754 floating point single precision */
    ia_mkn_dfid_double,                    /*!< IEEE-754 floating point double precision */

    /*! Fixed point fractional data types */
    ia_mkn_dfid_unsigned_q16_16,            /*!< 32-bit, Unsigned fixed point fractional value,  16 bits integer, 16 bits fractional */
    ia_mkn_dfid_signed_q15_16,              /*!< 32-bit, Signed fixed point fractional value,  1 bit sign, 15 bits integer, 16 bits fractional */
    ia_mkn_dfid_unsigned_q8_8,              /*!< 16-bit, Unsigned fixed point fractional value,  8 bits integer, 8 bits fractional */
    ia_mkn_dfid_signed_q7_8,                /*!< 16-bit, Signed fixed point fractional value,  1 bit sign, 7 bits integer, 8 bits fractional */

    ia_mkn_dfid_last                        /*!< Total number of data types, keep this enum member as a last one!. */
} ia_mkn_dfid;


/*!
 * \brief Data Name ID (DNID) enumeration describes a name (or functionality) of data in the 'Data' field.
 */
typedef enum
{
    ia_mkn_dnid_dummy,                      /*!< Used for dummy records (no actual data). */
    ia_mkn_dnid_ia_aiq_records = 1,         /*!< ia_aiq library internal records. */
    ia_mkn_dnid_hal_records = 256,          /*!< HAL records. */
    ia_mkn_dnid_ia_isp_records = 512,       /*!< ia_isp library internal records. */
    ia_mkn_dnid_free_records = 768,         /*!< Next free block of records - replace this when assigned. */
    ia_mkn_dnid_last                        /*!< Total number of data names, keep this enum member as a last one!. */
} ia_mkn_dnid;


/*!
 * \brief Bitfield to enable makernote features.
 */
typedef enum
{
    ia_mkn_cfg_compression  = 1,        /*!< Enable compression of maker note data */
    ia_mkn_cfg_imported  = 1 << 1,      /*!< Shows that MKN has been imported from binary container */
} ia_mkn_config_bits;


/*!
 * \brief Target of the makernote data: Section 1 or 2 (which could represent e.g. JPEG EXIF or RAW Header data).
 *  Notice that if Section 2 is selected, an output makernote data will contain both Section 1 and Section 2 parts.
 *  Bitwise OR'd with DNID when set for the target sections.
 */
typedef enum
{
    ia_mkn_trg_section_1    = 0,        /*!< Extensions ('or'-ed to ia_mkn_dnid) */
    ia_mkn_trg_section_2    = 0x10000   /*!< Extensions ('or'-ed to ia_mkn_dnid) */
} ia_mkn_trg;


/*!
 * \brief Makernote header structure. Records (ia_mkn_record_header) are followed after this header.
 */
typedef struct
{
    uint32_t tag;                           /*!< Tag in the beginning of makernote data. It also can be used to determine endianness. */
    uint32_t size;                          /*!< Size of the actual makernote records data (including ia_mkn_header) */
    uint32_t system_version;                /*!< Version of makernote system, format 0xYYMMDDVV */
    uint32_t enum_revision;                 /*!< Revision of makernote enumerations set, format 0xYYMMDDVV */
    uint32_t config_bits;                   /*!< Configuration flag bits set */
    uint32_t checksum;                      /*!< Global checksum of all bytes from the maker note buffer */
} ia_mkn_header;


/*!
 * \brief Record header structure. Data is followed after this header.
 */
typedef struct
{
    uint32_t size;                          /*!< Size of record including header */
    uint8_t data_format_id;                 /*!< ia_mkn_dfid enumeration values used */
    uint8_t key_id;                         /*!< Packing key ID. If 0 - no packing */
    uint16_t data_name_id;                  /*!< ia_mkn_dnid enumeration values used */
} ia_mkn_record_header;

#ifdef __cplusplus
}
#endif

#endif /* _IA_MKN_TYPES_H_ */
