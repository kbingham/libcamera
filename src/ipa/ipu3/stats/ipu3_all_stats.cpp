/*
 * Copyright (C) 2017-2018 Intel Corporation.
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
 *
 * IPU3AllStats - Handle and convert statistics from kernel to AIQ interface
 *
 * This implementation is highly derived from ChromeOS:
 *   platform2/camera/hal/intel/ipu3/psl/ipu3/statsConverter/ipu-stats.cpp
 */

#include "ipu3_all_stats.h"

#include <assert.h>
#include <string.h>

namespace libcamera {

#define IA_CSS_ENTER(...) \
	{                 \
	}
#define IA_CSS_LEAVE(...) \
	{                 \
	}
#define IA_CSS_ENTER_PRIVATE(...) \
	{                         \
	}
#define IA_CSS_LEAVE_PRIVATE(...) \
	{                         \
	}

typedef void *hrt_vaddress;

#define CSS_ALIGN(d, a) d __attribute__((aligned(a)))

#define HIVE_ISP_DDR_WORD_BITS 256
#define HIVE_ISP_DDR_WORD_BYTES (HIVE_ISP_DDR_WORD_BITS / 8)
#define RES_MGR_PRIVATE_MAX_NUM_OF_STRIPES 2

#define MAX_BUBBLE_SIZE 10
#define AWB_MD_ITEM_SIZE_IN_BYTES 8
#define AF_MD_ITEM_SIZE_IN_BYTES 4
#define AWB_FR_MD_ITEM_SIZE_IN_BYTES 8
#define AWB_MAX_NUM_OF_SETS 60
#define AWB_SIZE_OF_ONE_SET_IN_BYTES 0x500
#define AWB_SPARE_FOR_BUBBLES (MAX_BUBBLE_SIZE * RES_MGR_PRIVATE_MAX_NUM_OF_STRIPES * AWB_MD_ITEM_SIZE_IN_BYTES)
#define AF_MAX_NUM_OF_SETS 24
#define AF_SIZE_OF_ONE_Y_TABLE_SET_IN_BYTES 0x80
#define AF_SPARE_FOR_BUBBLES (MAX_BUBBLE_SIZE * RES_MGR_PRIVATE_MAX_NUM_OF_STRIPES * AF_MD_ITEM_SIZE_IN_BYTES)
#define AWB_FR_MAX_NUM_OF_SETS 24
#define AWB_FR_SIZE_OF_ONE_BAYER_TBL_IN_BYTES 0x100
#define AWB_FR_SPARE_FOR_BUBBLES (MAX_BUBBLE_SIZE * RES_MGR_PRIVATE_MAX_NUM_OF_STRIPES * AWB_FR_MD_ITEM_SIZE_IN_BYTES)

#define AE_PRIVATE_NUM_OF_HIST_BINS 256
#define AE_PRIVATE_NUM_OF_COLORS 4

#define AWB_MAX_BUFFER_SIZE (AWB_MAX_NUM_OF_SETS * (AWB_SIZE_OF_ONE_SET_IN_BYTES + AWB_SPARE_FOR_BUBBLES))
#define AF_MAX_SIZE_OF_Y_TABLE (AF_MAX_NUM_OF_SETS * (AF_SIZE_OF_ONE_Y_TABLE_SET_IN_BYTES + AF_SPARE_FOR_BUBBLES) * \
				RES_MGR_PRIVATE_MAX_NUM_OF_STRIPES)
#define AWB_FR_MAX_SIZE_OF_BAYER_TABLE (AWB_FR_MAX_NUM_OF_SETS *                                             \
					(AWB_FR_SIZE_OF_ONE_BAYER_TBL_IN_BYTES + AWB_FR_SPARE_FOR_BUBBLES) * \
					RES_MGR_PRIVATE_MAX_NUM_OF_STRIPES)

struct bubble_info {
	CSS_ALIGN(unsigned int num_of_stripes, HIVE_ISP_DDR_WORD_BYTES);
	CSS_ALIGN(unsigned int num_sets, HIVE_ISP_DDR_WORD_BYTES);
	CSS_ALIGN(unsigned int size_of_set, HIVE_ISP_DDR_WORD_BYTES);
	CSS_ALIGN(unsigned int bubble_size, HIVE_ISP_DDR_WORD_BYTES);
};

struct stats_3a_bubble_info_per_stripe {
	struct bubble_info awb_bubble_info[RES_MGR_PRIVATE_MAX_NUM_OF_STRIPES];
	struct bubble_info af_bubble_info[RES_MGR_PRIVATE_MAX_NUM_OF_STRIPES];
	struct bubble_info awb_fr_bubble_info[RES_MGR_PRIVATE_MAX_NUM_OF_STRIPES];
};

struct ff_status {
	CSS_ALIGN(unsigned int awb_en, HIVE_ISP_DDR_WORD_BYTES);
	CSS_ALIGN(unsigned int ae_en, HIVE_ISP_DDR_WORD_BYTES);
	CSS_ALIGN(unsigned int af_en, HIVE_ISP_DDR_WORD_BYTES);
	CSS_ALIGN(unsigned int awb_fr_en, HIVE_ISP_DDR_WORD_BYTES);
};

typedef struct awb_fr_private_config_s {
#if defined(__SP)
	unsigned int config_overlay[7];
#else
	struct {
		uint32_t grid_width : 6;
		uint32_t rsvd_0 : 2;
		uint32_t grid_height : 6;
		uint32_t rsvd_1 : 2;
		uint32_t block_width : 3;
		uint32_t block_height : 3;
		uint32_t grid_height_per_slice : 8;
		uint32_t rsvd_2 : 2;
	} bayer_grd_cfg;

	struct {
		uint32_t x_start : 12;
		uint32_t revd_0 : 4;
		uint32_t y_start : 12;
		uint32_t rsvd_1 : 3;
		uint32_t af_bayer_en : 1;
	} bayer_grd_start;

	struct {
		uint32_t x_end : 12;
		uint32_t rsvd_0 : 4;
		uint32_t y_end : 12;
		uint32_t rsvd_1 : 4;
	} bayer_grd_end;

	struct {
		uint32_t A1 : 8;
		uint32_t A2 : 8;
		uint32_t A3 : 8;
		uint32_t A4 : 8;
	} bayer_coeff_0;

	struct {
		uint32_t A5 : 8;
		uint32_t A6 : 8;
		uint32_t rsvd_0 : 16;
	} bayer_coeff_1;

	struct {
		uint32_t bayer_sign : 11;
		uint32_t rsvd_0 : 21;
	} bayer_sign_0;

	struct {
		uint32_t bayer_nf : 4;
		uint32_t rsvd_0 : 28;
	} nf;
#endif
} awb_fr_private_config_t;

typedef struct {
#if defined(__SP)
	unsigned int config_overlay[3];
#else
	struct {
		unsigned int grid_width : 6;
		unsigned int spare0 : 2;
		unsigned int grid_height : 6;
		unsigned int spare1 : 2;
		unsigned int block_width : 3;
		unsigned int block_height : 3;
		unsigned int grid_height_per_slice : 8; /* default value 1 */
		unsigned int spare2 : 2;

	} grd_cfg;

	struct {
		unsigned int x_start : 12; /* default 10 */
		unsigned int spare0 : 4;
		unsigned int y_start : 12; /* default 2 */
		unsigned int spare1 : 3;
		unsigned int en : 1; /* default: 1 */
	} grd_start;

	struct {
		unsigned int x_end : 12;
		unsigned int spare0 : 4;
		unsigned int y_end : 12;
		unsigned int spare1 : 4;
	} grd_end;
#endif
} af_private_ff_af_grid_config_t;

struct af_frame_size {
	/* Represents the number of pixels in a row. NOF_Col == frame_width */
	unsigned int nof_col : 16;
	/* Represents the number of rows in a frame. NOF_Row == frame_height */
	unsigned int nof_row : 16;
};

typedef struct {
#if defined(__SP)

	unsigned int config_overlay[11];

#else

	struct {
		unsigned char A1;
		unsigned char A2;
		unsigned char A3;
		unsigned char A4;
	} y1_coeff_0;
	struct {
		unsigned char A5;
		unsigned char A6;
		unsigned char A7;
		unsigned char A8;
	} y1_coeff_1;
	struct {
		unsigned char A9;
		unsigned char A10;
		unsigned char A11;
		unsigned char A12;
	} y1_coeff_2;

	unsigned int y1_sign_vec;

	struct {
		unsigned char A1;
		unsigned char A2;
		unsigned char A3;
		unsigned char A4;
	} y2_coeff_0;
	struct {
		unsigned char A5;
		unsigned char A6;
		unsigned char A7;
		unsigned char A8;
	} y2_coeff_1;
	struct {
		unsigned char A9;
		unsigned char A10;
		unsigned char A11;
		unsigned char A12;
	} y2_coeff_2;

	unsigned int y2_sign_vec;

	struct {
		unsigned int y_gen_rate_gr : 6;
		unsigned int spare0 : 2;
		unsigned int y_gen_rate_r : 6;
		unsigned int spare1 : 2;
		unsigned int y_gen_rate_b : 6;
		unsigned int spare2 : 2;
		unsigned int y_gen_rate_gb : 6;
		unsigned int spare3 : 2;
	} y_calc;

	struct {
		unsigned int spare0 : 8;
		unsigned int y1_nf : 4;
		unsigned int spare1 : 4;
		unsigned int y2_nf : 4;
		unsigned int spare2 : 12;
	} nf;

	struct af_frame_size frame_size;

#endif
} af_private_ff_3a_fltr_rspns_config_t;

/*
 * this type is mirroring portions of the internal ff mmio,
 * the portions are used for configuration
 */

typedef struct af_private_config_s {
	CSS_ALIGN(af_private_ff_3a_fltr_rspns_config_t ff_3a_fltr_rspns_config,
		  HIVE_ISP_DDR_WORD_BYTES);

	CSS_ALIGN(af_private_ff_af_grid_config_t y_grid_config,
		  HIVE_ISP_DDR_WORD_BYTES);

} af_private_config_t;

// this type is mirroring the internal ff mmio
typedef struct awb_private_config_s {
#if defined(__SP)
	// hivecc doesn't support the sizeof operator with bit-fields
	// so we work around that with a separate definition for sp
	unsigned int config_overlay[5];
#else
	struct {
		unsigned int rgbs_thr_gr : 13;
		unsigned int spr0 : 3;
		unsigned int rgbs_thr_r : 13;
		unsigned int spr1 : 3;
	} rgbs_thrsh_0;

	struct {
		unsigned int rgbs_thr_gb : 13;
		unsigned int spr0 : 3;
		unsigned int rgbs_thr_b : 13;
		unsigned int spr1 : 1;
		unsigned int rgbs_en : 1; //controls generation of metat_data (like FF enable/disable) and not as discribed in HAS
		unsigned int rgbs_incl_sat : 1;
	} rgbs_thrsh_1;

	struct {
		unsigned int grid_width : 7;
		unsigned int spr0 : 1;
		unsigned int grid_height : 7;
		unsigned int spr1 : 1;
		unsigned int block_width : 3;
		unsigned int block_height : 3;
		unsigned int grid_height_per_slice : 8;
		unsigned int spr2 : 2;
	} rgbs_grd_cfg;

	struct {
		unsigned int x_start : 12;
		unsigned int spr0 : 4;
		unsigned int y_start : 12;
		unsigned int spr1 : 4;
	} rgbs_grd_start;

	struct {
		unsigned int x_end : 12;
		unsigned int spr0 : 4;
		unsigned int y_end : 12;
		unsigned int spr1 : 4;
	} rgbs_grd_end;
#endif
} awb_private_config_t;

typedef struct ae_private_direct_config_s {
#if defined(__SP)
	unsigned int config_overlay[3];
#else
	unsigned int grid_width : 8;
	unsigned int grid_height : 8;
	unsigned int block_width : 4;
	unsigned int block_height : 4;
	unsigned int spare0 : 5;
	unsigned int AE_En : 1;
	unsigned int rst_hist_array : 1;
	unsigned int done_rst_hist_array : 1;

	unsigned int x_start : 12;
	unsigned int spare1 : 4;
	unsigned int y_start : 12;
	unsigned int spare2 : 4;

	unsigned int x_end : 12;
	unsigned int spare3 : 4;
	unsigned int y_end : 12;
	unsigned int spare4 : 4;
#endif
} ae_private_direct_config_t;

struct ia_css_4a_private_config {
	CSS_ALIGN(struct awb_private_config_s awb_config,
		  HIVE_ISP_DDR_WORD_BYTES);

	CSS_ALIGN(struct ae_private_direct_config_s ae_grd_config,
		  HIVE_ISP_DDR_WORD_BYTES);

	CSS_ALIGN(struct af_private_config_s af_config,
		  HIVE_ISP_DDR_WORD_BYTES);

	CSS_ALIGN(struct awb_fr_private_config_s awb_fr_config,
		  HIVE_ISP_DDR_WORD_BYTES);
};

typedef struct {
	unsigned int vals[AE_PRIVATE_NUM_OF_HIST_BINS * AE_PRIVATE_NUM_OF_COLORS];
} ae_private_raw_buffer_t;

typedef struct {
	CSS_ALIGN(ae_private_raw_buffer_t buff,
		  HIVE_ISP_DDR_WORD_BYTES);
} ae_private_raw_buffer_aligned_t;

typedef struct {
	unsigned char meta_data_buffer[AWB_MAX_BUFFER_SIZE];
} awb_private_meta_data_t;

/*
 * the sp layer of fills this struct during frame processing
 * it's allocated by another layer/component
 * the address is communicated to the awb sp layer
 * via a getter function
 */
typedef struct {
	awb_private_meta_data_t meta_data;
} awb_private_raw_buffer_t;

typedef struct {
	CSS_ALIGN(unsigned char y_table[AF_MAX_SIZE_OF_Y_TABLE], HIVE_ISP_DDR_WORD_BYTES);
} af_private_meta_data_t;

/*
 * the sp layer of the af component fills this struct during frame processing
 * it's allocated by another layer/component
 * the address is communicated to the sp layer of the af component
 * via a getter function
 */
typedef struct {
	CSS_ALIGN(af_private_meta_data_t meta_data, HIVE_ISP_DDR_WORD_BYTES);
} af_private_raw_buffer_t;

typedef struct {
	CSS_ALIGN(uint8_t bayer_table[AWB_FR_MAX_SIZE_OF_BAYER_TABLE], HIVE_ISP_DDR_WORD_BYTES);
} awb_fr_private_meta_data_t;

typedef struct {
	awb_fr_private_meta_data_t meta_data;
} awb_fr_private_raw_buffer_t;

struct stats_4a_private_raw_buffer {
	CSS_ALIGN(awb_private_raw_buffer_t awb_raw_buffer,
		  HIVE_ISP_DDR_WORD_BYTES);

	CSS_ALIGN(ae_private_raw_buffer_aligned_t ae_raw_buffer[RES_MGR_PRIVATE_MAX_NUM_OF_STRIPES],
		  HIVE_ISP_DDR_WORD_BYTES);

	CSS_ALIGN(af_private_raw_buffer_t af_raw_buffer,
		  HIVE_ISP_DDR_WORD_BYTES);

	CSS_ALIGN(awb_fr_private_raw_buffer_t awb_fr_raw_buffer,
		  HIVE_ISP_DDR_WORD_BYTES);

	CSS_ALIGN(struct ia_css_4a_private_config stats_4a_config,
		  HIVE_ISP_DDR_WORD_BYTES);

	CSS_ALIGN(unsigned int ae_join_buffers,
		  HIVE_ISP_DDR_WORD_BYTES);

	CSS_ALIGN(struct stats_3a_bubble_info_per_stripe stats_3a_bubble_per_stripe,
		  HIVE_ISP_DDR_WORD_BYTES);

	CSS_ALIGN(struct ff_status stats_3a_status,
		  HIVE_ISP_DDR_WORD_BYTES);
};

static void ia_css_3a_join_ae_buffers(ae_public_raw_buffer_t *to,
				      ae_private_raw_buffer_aligned_t *ae_buff)
{
	unsigned int i = 0, j = 0;
	unsigned int size = AE_PRIVATE_NUM_OF_HIST_BINS * AE_PRIVATE_NUM_OF_COLORS;
	unsigned int color = 0;

	IA_CSS_ENTER_PRIVATE("to=%p, from=%p", to, ae_buff);

	for (i = 0; i < size; i++) {
		color = i / AE_PRIVATE_NUM_OF_HIST_BINS;
		j = i % AE_PRIVATE_NUM_OF_HIST_BINS;

		switch (color) {
		case 0: {
			to->hist_R.vals[j] += ae_buff->buff.vals[i];
			break;
		}
		case 1: {
			to->hist_G.vals[j] += ae_buff->buff.vals[i];
			break;
		}
		case 2: {
			to->hist_B.vals[j] += ae_buff->buff.vals[i];
			break;
		}
		case 3: {
			to->hist_Y.vals[j] += ae_buff->buff.vals[i];
			break;
		}
		default: {
			assert(false);
			break;
		}
		}
	}
	IA_CSS_LEAVE_PRIVATE("");
}

static void mmgr_load(void *src, void *dst, int len)
{
	/* Wrapper imported from ipu3 IPA in CrOS. Originally was MEMCPY_S.
	 * \todo: Check whether we need to use memcpy_s() instead? */
	memcpy(dst, src, (size_t)(len));
}

void IPU3AllStats::ipu3_stats_get_3a([[maybe_unused]] struct ipu3_stats_all_stats *all_stats,
				     [[maybe_unused]] const struct ipu3_uapi_stats_3a *isp_stats)
{
	/* extract, memcpy and debubble each of 3A stats */
	struct ia_css_4a_statistics *host_stats = &all_stats->ia_css_4a_statistics;
	struct ia_css_4a_private_config stats_config;
	ae_private_raw_buffer_aligned_t ae_raw_buffer_s;
	unsigned int ae_join_buffers;

	hrt_vaddress af_ddr_addr = (hrt_vaddress)(long int)&(((struct stats_4a_private_raw_buffer *)(long int)isp_stats)->af_raw_buffer);

	hrt_vaddress awb_ddr_addr = (hrt_vaddress)(long int)&((struct stats_4a_private_raw_buffer *)(long int)isp_stats)->awb_raw_buffer;

	hrt_vaddress awb_fr_ddr_addr = (hrt_vaddress)(long int)&((struct stats_4a_private_raw_buffer *)(long int)isp_stats)->awb_fr_raw_buffer;

	hrt_vaddress ae_buff_0_ddr_addr = (hrt_vaddress)(long int)&((struct stats_4a_private_raw_buffer *)(long int)isp_stats)->ae_raw_buffer[0];
	hrt_vaddress ae_buff_1_ddr_addr = (hrt_vaddress)(long int)&((struct stats_4a_private_raw_buffer *)(long int)isp_stats)->ae_raw_buffer[1];

	hrt_vaddress ae_pp_info_addr = (hrt_vaddress)(long int)&((struct stats_4a_private_raw_buffer *)(long int)isp_stats)->ae_join_buffers;

	hrt_vaddress stats_config_addr = (hrt_vaddress) & (isp_stats->stats_4a_config);

	/* load grid configuration */
	mmgr_load(stats_config_addr,
		  (void *)&(stats_config),
		  sizeof(struct ia_css_4a_private_config));

	/* load ae post processing info */
	mmgr_load(ae_pp_info_addr,
		  (void *)&(ae_join_buffers),
		  sizeof(unsigned int));

	/* load metadata */
	mmgr_load(af_ddr_addr,
		  (void *)&(host_stats->data->af_raw_buffer),
		  sizeof(af_public_raw_buffer_t));

	mmgr_load(awb_ddr_addr,
		  (void *)&(host_stats->data->awb_raw_buffer),
		  sizeof(awb_public_raw_buffer_t));

	mmgr_load(awb_fr_ddr_addr,
		  (void *)&(host_stats->data->awb_fr_raw_buffer),
		  sizeof(awb_fr_public_raw_buffer_t));

	mmgr_load(ae_buff_0_ddr_addr,
		  (void *)&(host_stats->data->ae_raw_buffer),
		  sizeof(ae_public_raw_buffer_t));

	if (ae_join_buffers == 1) {
		mmgr_load(ae_buff_1_ddr_addr,
			  (void *)&(ae_raw_buffer_s),
			  sizeof(ae_private_raw_buffer_aligned_t));
	}

	/* for striping might need to combine buffers of ae */
	if (ae_join_buffers == 1)
		ia_css_3a_join_ae_buffers(&host_stats->data->ae_raw_buffer,
					  &ae_raw_buffer_s);
}

ia_err
IPU3AllStats::intel_skycam_statistics_convert(const ia_css_4a_statistics &statistics,
					      ia_aiq_rgbs_grid *out_rgbs_grid,
					      ia_aiq_af_grid *out_af_grid)
{
	if (!out_rgbs_grid || !out_af_grid) {
		return ia_err_data;
	}

	// AWB (RGBS) grid.
	out_rgbs_grid->grid_width = statistics.stats_4a_config->awb_grd_config.grid_width;
	out_rgbs_grid->grid_height = statistics.stats_4a_config->awb_grd_config.grid_height;

	for (int i = 0; i < out_rgbs_grid->grid_width * out_rgbs_grid->grid_height; ++i) {
		out_rgbs_grid->blocks_ptr[i].avg_r = statistics.data->awb_raw_buffer.rgb_table[i].R_avg;
		out_rgbs_grid->blocks_ptr[i].avg_b = statistics.data->awb_raw_buffer.rgb_table[i].B_avg;
		out_rgbs_grid->blocks_ptr[i].avg_gb = statistics.data->awb_raw_buffer.rgb_table[i].Gb_avg;
		out_rgbs_grid->blocks_ptr[i].avg_gr = statistics.data->awb_raw_buffer.rgb_table[i].Gr_avg;
		out_rgbs_grid->blocks_ptr[i].sat = statistics.data->awb_raw_buffer.rgb_table[i].sat_ratio;
	}

	// AF (aka F response) grid.
	out_af_grid->grid_width = statistics.stats_4a_config->af_grd_config.grid_width;
	out_af_grid->grid_height = statistics.stats_4a_config->af_grd_config.grid_height;

	// The AIQ block expects block dimensions specified in BQ's, while
	// SkyCam uses log2 of pixel count.
	out_af_grid->block_width = 1 << (statistics.stats_4a_config->af_grd_config.block_width - 1);
	out_af_grid->block_height = 1 << (statistics.stats_4a_config->af_grd_config.block_height - 1);

	for (int i = 0; i < out_af_grid->grid_width * out_af_grid->grid_height; ++i) {
		out_af_grid->filter_response_1[i] = statistics.data->af_raw_buffer.y_table[i].y1_avg;
		out_af_grid->filter_response_2[i] = statistics.data->af_raw_buffer.y_table[i].y2_avg;
	}

	return ia_err_none;
}

} // namespace libcamera

