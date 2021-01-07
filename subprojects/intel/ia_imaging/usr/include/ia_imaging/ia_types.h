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

#ifndef _IA_TYPES_H_
#define _IA_TYPES_H_

#include <stdlib.h>
#include <stdarg.h>

#include <stdbool.h>

#define LIBEXPORT
#ifdef __cplusplus
extern "C" {
#endif

/**
 *  \brief Bundles binary data pointer with size.
 */
typedef struct
{
    void        *data;
    unsigned int size;
} ia_binary_data; /* ia_binay_data owns data */

/**
 *  \brief Defines a common record header.
 */
typedef struct
{
    unsigned int uuid;
    unsigned int size;
} ia_record_header;

typedef struct {
    int major;
    int minor;
} ia_version;

typedef enum {
    CAPTURE_OUTPUT,
    CAPTURE_VFPP,
    PREVIEW_VFPP,
    ACC_QOS
} ia_acceleration_fw_dst;

typedef struct {
    void *isp;
    void *(*open_firmware)     (const char *fw_path, unsigned *size);
    int   (*load_firmware)     (void *isp, void *fw, unsigned size, unsigned *handle);
    int   (*unload_firmware)   (void *isp, unsigned handle);
    int   (*map_firmware_arg)  (void *isp, void *usr_ptr, size_t size, unsigned long *css_ptr);
    int   (*unmap_firmware_arg)(void *isp, unsigned long css_ptr, size_t size);
    int   (*set_firmware_arg)  (void *isp, unsigned handle, unsigned num, void *val, size_t size);
    int   (*set_mapped_arg)    (void *isp, unsigned handle, unsigned mem, unsigned long css_ptr, size_t size);
    int   (*start_firmware)    (void *isp, unsigned handle);
    int   (*wait_for_firmware) (void *isp, unsigned handle);
    int   (*abort_firmware)    (void *isp, unsigned handle, unsigned timeout);
    ia_version version_isp;
    ia_version version_css;
    int   (*set_stage_state)   (void *isp, unsigned int handle, bool enable);
    int   (*wait_stage_update) (void *isp, unsigned int handle);
    int   (*load_firmware_ext) (void *isp, void *fw, unsigned size, unsigned *handle, int fw_dst);
} ia_acceleration;

typedef enum {
    ia_frame_format_nv12,       /* 12 bit YUV 420, Y, UV plane */
    ia_frame_format_yuv420,     /* 12 bit YUV 420, Y, U, V plane */
    ia_frame_format_yuv444,     /* 24 bit YUV 444, Y, U, V plane */
    ia_frame_format_raw,        /* RAW, 1 plane */
    ia_frame_format_rgba32,     /* RGBA 8 bits per channel */
    ia_frame_format_yuy2,       /* 16 bit YUV 422, YUYV plane */
    ia_frame_format_raw16,      /* 16 bit RAW, 1 plane */
    ia_frame_format_rgb16p,     /* 16 bits per channel, 3 planes */
    ia_frame_format_yuv420sp16, /* 24 bit YUV 420, Y, UV plane */
    ia_frame_format_yuv420p16,  /* 24 bit YUV 420, Y, U, V plane */
    ia_frame_format_yuv444p16,  /* 48 bit YUV 444, Y, U, V plane */
} ia_frame_format;

typedef struct {
    void           *data;     /**< Pointer to the image data, ia_ipf_buffer_container owns this */
    int             size;     /**< Total number of bytes in data*/
    int             width;    /**< Width of the frame in pixels */
    int             height;   /**< Height of the frame in lines */
    ia_frame_format format;   /**< Frame format */
    int             stride;   /**< Stride, bytes per line*/
    int             rotation; /**< degrees 0-360 */
} ia_frame;

/*!
* \brief Raw image data format.
*/
typedef enum
{
    ia_image_data_format_rawplain16,             /*!< Data is 16 bits per pixel. */
    ia_image_data_format_rawplain8,              /*!< Data is 8 bits per pixel. */
    ia_image_data_format_rgb16,                  /*!< Data is in a buffer contaning planes: R, G and B one after another. Data size is 16 bits per pixel. */
    ia_image_data_format_smia10,                 /*!< SMIA 10 format groups 4 pixels: MSBs of four pixels in 4 bytes + LSBs of same 4 pixels in 1 byte. */
    ia_image_data_format_nv12,                   /*!< Data is 12 bit YUV 420, Y, UV plane. */
    ia_image_data_format_yuv420_planar,          /*!< Data is YUV 420 planar. */
    ia_image_data_format_rawplain16_interleaved, /*!< Data is 16 bits per pixel interleaved so that all bayer quad pixels are one after another. Example: GRBGGRBG...*/
} ia_image_data_format;

/*!
* \brief Raw image essential information.
* If raw image data doesn't contain any extra bytes/lines/columns, the image can be decoded with this info.
* For all signed values in the structs below, negative value indicates N/A.
*/
typedef struct
{
    unsigned char data_format;   /*!< RawPlain16, RawPlain8 etc. as defined in the enum ia_image_data_format */
    char bayer_order;            /*!< GRBG, RGGB, BGGR or GBRG as defined in the enum cmc_bayer_order. -1 if N/A */
    char data_format_bpp;        /*!< Bits per pixel for a given data_format. eg. 16 for RawPlain16 etc. -1 if N/A*/
    char data_bpp;               /*!< Bits per pixel used by the image data. -1 if N/A */
    unsigned int width_cols;     /*!< Visible image width (640, 800, 1280, 1600, etc.) */
    unsigned int height_lines;   /*!< Visible image height (480, 600, 960, 1200, etc.) */
} ia_image_info;

/*!
* \brief Raw image info with additional details about extra bytes/lines/columns to skip when decoding the raw image.
*/
typedef struct
{
    ia_image_info raw_image;            /*!< Essential information needed to decode the image. */
    unsigned int header_size_bytes;     /*!< Variable size header */
    unsigned int footer_size_bytes;     /*!< Variable size footer */

    unsigned short extra_bytes_left;    /*!< e.g. CCP sync codes */
    unsigned short extra_bytes_right;   /*!< e.g. CCP sync codes */

    unsigned short extra_lines_top;     /*!< Non-image data, e.g. embedded data lines */
    unsigned short extra_cols_left;     /*!< Non-image data, extra columns at left */
    unsigned short extra_cols_right;    /*!< Non-image data, extra columns at right */
    unsigned short extra_lines_bottom;  /*!< Non-image data, e.g. embedded data lines */

    char byte_order_xor;                /*!< 0, 1 or 3 (for the cases when byte order has been modified for whatever reason) */
    char spatial_sampling;              /*!< 0 or 1 = Bayer or Co-sited */
} ia_image_full_info;

/** Floating point range struct. */
typedef struct {
    float min;
    float max;
} ia_float_range;

/*!
 * Needs to be agreed with ia_imaging error codes.
 * \brief Error codes.
*/
typedef enum {
    ia_err_none     =  0,       /*!< No errors*/
    ia_err_general  = (1 << 1), /*!< General error*/
    ia_err_nomemory = (1 << 2), /*!< Out of memory*/
    ia_err_data     = (1 << 3), /*!< Corrupted data*/
    ia_err_internal = (1 << 4), /*!< Error in code*/
    ia_err_argument = (1 << 5), /*!< Invalid argument for a function*/
    ia_err_disabled = (1 << 6)  /*!< Functionality is disabled*/
} ia_err;

typedef struct {
    int left;
    int top;
    int right;
    int bottom;
} ia_rectangle;

/** Vector, used for image stabilization. */
typedef struct {
    int x;
    int y;
} ia_vector;

/** Coordinate, used in red-eye correction. */
typedef struct {
    int x;
    int y;
} ia_coordinate;

typedef struct {
    void (*vdebug)(const char *fmt, ...);
    void (*verror)(const char *fmt, ...);
    void (*vinfo)(const char *fmt, ...);
} ia_env;

typedef struct {
    void *(*alloc) (void *isp, size_t size);
    void  (*free)  (void *isp, void *usr_ptr, size_t size);
} ia_mem_env;

#ifdef __cplusplus
}
#endif

#endif /* _IA_TYPES_H_ */
