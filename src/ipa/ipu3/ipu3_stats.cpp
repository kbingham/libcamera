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
 * IPAIPU3Stats - Handle and convert statistics from kernel to AIQ interface
 *
 * This implementation is highly derived from ChromeOS:
 *   platform2/camera/hal/intel/ipu3/psl/ipu3/statsConverter/ipu-stats.cpp
 */

#include "ipu3_stats.h"

#include <assert.h>
#include <string.h>

namespace libcamera {

#define IA_CSS_ENTER(...) { }
#define IA_CSS_LEAVE(...) { }
#define IA_CSS_ENTER_PRIVATE(...) { }
#define IA_CSS_LEAVE_PRIVATE(...) { }
#define ia_css_debug_dtrace(...) { }

typedef void * hrt_vaddress;

#define CSS_ALIGN(d, a) d __attribute__((aligned(a)))

#define HIVE_ISP_DDR_WORD_BITS   256
#define HIVE_ISP_DDR_WORD_BYTES  (HIVE_ISP_DDR_WORD_BITS/8)
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
#define AWB_FR_MAX_SIZE_OF_BAYER_TABLE (AWB_FR_MAX_NUM_OF_SETS *        \
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
        uint32_t  grid_width		:   6;
        uint32_t  rsvd_0		:   2;
        uint32_t  grid_height		:   6;
        uint32_t  rsvd_1		:   2;
        uint32_t  block_width		:   3;
        uint32_t  block_height		:   3;
        uint32_t  grid_height_per_slice	:   8;
        uint32_t  rsvd_2		:   2;
    } bayer_grd_cfg;

    struct {
        uint32_t  x_start		:  12;
        uint32_t  revd_0		:   4;
        uint32_t  y_start		:  12;
        uint32_t  rsvd_1		:   3;
        uint32_t  af_bayer_en		:   1;
    } bayer_grd_start;

    struct {
        uint32_t  x_end			:  12;
        uint32_t  rsvd_0		:   4;
        uint32_t  y_end			:  12;
        uint32_t  rsvd_1		:   4;
    } bayer_grd_end;

    struct {
        uint32_t  A1			:   8;
        uint32_t  A2			:   8;
        uint32_t  A3			:   8;
        uint32_t  A4			:   8;
    } bayer_coeff_0 ;

    struct {
        uint32_t  A5			:   8;
        uint32_t  A6			:   8;
        uint32_t  rsvd_0		:  16;
    }  bayer_coeff_1;

    struct {
        uint32_t  bayer_sign		:  11;
        uint32_t  rsvd_0		:  21;
    } bayer_sign_0;

    struct {
        uint32_t  bayer_nf		:   4;
        uint32_t  rsvd_0		:  28;
    } nf;
#endif
} awb_fr_private_config_t;

typedef struct {
#if defined(__SP)
    unsigned int config_overlay[3];
#else
    struct {
        unsigned int grid_width   : 6;
        unsigned int spare0       : 2;
        unsigned int grid_height  : 6;
        unsigned int spare1       : 2;
        unsigned int block_width  : 3;
        unsigned int block_height : 3;
        unsigned int grid_height_per_slice : 8; /* default value 1 */
        unsigned int spare2       : 2;

    } grd_cfg;

    struct {
        unsigned int x_start : 12; /* default 10 */
        unsigned int spare0  : 4;
        unsigned int y_start : 12; /* default 2 */
        unsigned int spare1  : 3;
        unsigned int en      : 1;  /* default: 1 */
    } grd_start;

    struct {
        unsigned int x_end  : 12;
        unsigned int spare0 : 4;
        unsigned int y_end  : 12;
        unsigned int spare1 : 4;
    } grd_end;
#endif
} af_private_ff_af_grid_config_t;

struct af_frame_size {
    /* Represents the number of pixels in a row. NOF_Col == frame_width */
    unsigned int nof_col  : 16;
    /* Represents the number of rows in a frame. NOF_Row == frame_height */
    unsigned int nof_row  : 16;
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
        unsigned int spare0        : 2;
        unsigned int y_gen_rate_r  : 6;
        unsigned int spare1        : 2;
        unsigned int y_gen_rate_b  : 6;
        unsigned int spare2        : 2;
        unsigned int y_gen_rate_gb : 6;
        unsigned int spare3        : 2;
    } y_calc;

    struct {
        unsigned int spare0   : 8;
        unsigned int y1_nf    : 4;
        unsigned int spare1   : 4;
        unsigned int y2_nf    : 4;
        unsigned int spare2   : 12;
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
        unsigned int rgbs_thr_gr   :13;
        unsigned int spr0          :3;
        unsigned int rgbs_thr_r    :13;
        unsigned int spr1          :3;
    } rgbs_thrsh_0;

    struct {
        unsigned int rgbs_thr_gb   :13;
        unsigned int spr0          :3;
        unsigned int rgbs_thr_b    :13;
        unsigned int spr1          :1;
        unsigned int rgbs_en       :1; //controls generation of metat_data (like FF enable/disable) and not as discribed in HAS
        unsigned int rgbs_incl_sat :1;
    } rgbs_thrsh_1;

    struct {
        unsigned int grid_width            :7;
        unsigned int spr0                  :1;
        unsigned int grid_height           :7;
        unsigned int spr1                  :1;
        unsigned int block_width           :3;
        unsigned int block_height          :3;
        unsigned int grid_height_per_slice :8;
        unsigned int spr2		   :2;
    } rgbs_grd_cfg;

    struct {
        unsigned int x_start		:12;
        unsigned int spr0		:4;
        unsigned int y_start		:12;
        unsigned int spr1		:4;
    } rgbs_grd_start;

    struct {
        unsigned int x_end		:12;
        unsigned int spr0		:4;
        unsigned int y_end		:12;
        unsigned int spr1		:4;
    } rgbs_grd_end;
#endif
} awb_private_config_t;

typedef struct ae_private_direct_config_s {
#if defined(__SP)
    unsigned int config_overlay[3];
#else
    unsigned int grid_width:8;
    unsigned int grid_height:8;
    unsigned int block_width:4;
    unsigned int block_height:4;
    unsigned int spare0:5;
    unsigned int AE_En:1;
    unsigned int rst_hist_array:1;
    unsigned int done_rst_hist_array:1;

    unsigned int x_start:12;
    unsigned int spare1:4;
    unsigned int y_start:12;
    unsigned int spare2:4;

    unsigned int x_end:12;
    unsigned int spare3:4;
    unsigned int y_end:12;
    unsigned int spare4:4;
#endif
} ae_private_direct_config_t;

struct ia_css_4a_private_config {
    CSS_ALIGN(struct awb_private_config_s		awb_config,
              HIVE_ISP_DDR_WORD_BYTES);

    CSS_ALIGN(struct ae_private_direct_config_s	ae_grd_config,
              HIVE_ISP_DDR_WORD_BYTES);

    CSS_ALIGN(struct af_private_config_s		af_config,
              HIVE_ISP_DDR_WORD_BYTES);

    CSS_ALIGN(struct awb_fr_private_config_s	awb_fr_config,
              HIVE_ISP_DDR_WORD_BYTES);
};

typedef struct {
    unsigned int vals[AE_PRIVATE_NUM_OF_HIST_BINS*AE_PRIVATE_NUM_OF_COLORS];
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
    CSS_ALIGN(unsigned char y_table  [AF_MAX_SIZE_OF_Y_TABLE], HIVE_ISP_DDR_WORD_BYTES);
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

    CSS_ALIGN(awb_private_raw_buffer_t		awb_raw_buffer,
              HIVE_ISP_DDR_WORD_BYTES);

    CSS_ALIGN(ae_private_raw_buffer_aligned_t	ae_raw_buffer[RES_MGR_PRIVATE_MAX_NUM_OF_STRIPES],
              HIVE_ISP_DDR_WORD_BYTES);

    CSS_ALIGN(af_private_raw_buffer_t		af_raw_buffer,
              HIVE_ISP_DDR_WORD_BYTES);

    CSS_ALIGN(awb_fr_private_raw_buffer_t		awb_fr_raw_buffer,
              HIVE_ISP_DDR_WORD_BYTES);

    CSS_ALIGN(struct ia_css_4a_private_config	stats_4a_config,
              HIVE_ISP_DDR_WORD_BYTES);

    CSS_ALIGN(unsigned int				ae_join_buffers,
              HIVE_ISP_DDR_WORD_BYTES);

    CSS_ALIGN(struct stats_3a_bubble_info_per_stripe	stats_3a_bubble_per_stripe,
              HIVE_ISP_DDR_WORD_BYTES);

    CSS_ALIGN(struct ff_status			stats_3a_status,
              HIVE_ISP_DDR_WORD_BYTES);


};

static void ia_css_3a_join_ae_buffers(ae_public_raw_buffer_t *to,
                                      ae_private_raw_buffer_aligned_t *ae_buff)
{
	unsigned int i = 0, j = 0;
	unsigned int size = AE_PRIVATE_NUM_OF_HIST_BINS * AE_PRIVATE_NUM_OF_COLORS;
	unsigned int color = 0;

	IA_CSS_ENTER_PRIVATE("to=%p, from=%p", to, ae_buff);

	for(i = 0; i < size; i++)
	{
		color = i / AE_PRIVATE_NUM_OF_HIST_BINS;
		j = i % AE_PRIVATE_NUM_OF_HIST_BINS;

		switch(color)
		{
			case 0:
			{
				to->hist_R.vals[j] += ae_buff->buff.vals[i];
				break;
			}
			case 1:
			{
				to->hist_G.vals[j] += ae_buff->buff.vals[i];
				break;
			}
			case 2:
			{
				to->hist_B.vals[j] += ae_buff->buff.vals[i];
				break;
			}
			case 3:
			{
				to->hist_Y.vals[j] += ae_buff->buff.vals[i];
				break;
			}
			default:
			{
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

static void ia_css_awb_grid_config_ddr_decode(struct awb_public_config_grid_config *to,
					      struct awb_private_config_s *from)
{
	IA_CSS_ENTER_PRIVATE("to=%p, from=%p", to, from);

	to->grid_height		= from->rgbs_grd_cfg.grid_height;
	to->grid_width		= from->rgbs_grd_cfg.grid_width;
	to->grid_x_start	= from->rgbs_grd_start.x_start;
	to->grid_y_start	= from->rgbs_grd_start.y_start;
	to->grid_block_width	= from->rgbs_grd_cfg.block_width;
	to->grid_block_height	= from->rgbs_grd_cfg.block_height;

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"\nW = %d, H = %d , BW = %d, BH = %d , X_S = %d, Y_S = %d \n",
		to->grid_width,
		to->grid_height,
		to->grid_block_width,
		to->grid_block_height,
		to->grid_x_start,
		to->grid_y_start);

	IA_CSS_LEAVE_PRIVATE("");
}

static void
ia_css_af_grid_config_ddr_decode(struct af_public_grid_config *to,
				 struct af_private_config_s *from)
{
	IA_CSS_ENTER_PRIVATE("to=%p, from=%p", to, from);

	to->grid_width	 = from->y_grid_config.grd_cfg.grid_width;
	to->grid_height	 = from->y_grid_config.grd_cfg.grid_height;
	to->block_width	 = from->y_grid_config.grd_cfg.block_width;
	to->block_height = from->y_grid_config.grd_cfg.block_height;
	to->x_start	 = from->y_grid_config.grd_start.x_start;
	to->y_start	 = from->y_grid_config.grd_start.y_start;


	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"\nW = %d, H = %d , BW = %d, BH = %d , X = %d, Y = %d \n",
		to->grid_width,
		to->grid_height,
		to->block_width,
		to->block_height,
		to->x_start,
		to->y_start);

	IA_CSS_LEAVE_PRIVATE("");
}

static void ia_css_awb_fr_grid_config_ddr_decode(struct awb_fr_public_grid_config *to,
						 struct awb_fr_private_config_s *from)
{

	IA_CSS_ENTER_PRIVATE("to=%p, from=%p", to, from);

	to->grid_width   = from->bayer_grd_cfg.grid_width;
	to->grid_height  = from->bayer_grd_cfg.grid_height;
	to->block_width  = from->bayer_grd_cfg.block_width;
	to->block_height = from->bayer_grd_cfg.block_height;
	to->x_start      = from->bayer_grd_start.x_start;
	to->y_start      = from->bayer_grd_start.y_start;

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"\nW = %d, H = %d , BW = %d, BH = %d , X = %d, Y = %d \n",
		to->grid_width,
		to->grid_height,
		to->block_width,
		to->block_height,
		to->x_start,
		to->y_start);

	IA_CSS_LEAVE_PRIVATE("");
}

static void
ia_css_ae_grid_config_ddr_decode(struct ae_public_config_grid_config *to,
				 struct ae_private_direct_config_s *from)
{
	IA_CSS_ENTER_PRIVATE("to=%p, from=%p", to, from);

	to->grid_width	 = from->grid_width;
	to->grid_height	 = from->grid_height;
	to->block_width	 = from->block_width;
	to->block_height = from->block_height;
	to->x_start	 = from->x_start;
	to->y_start	 = from->y_start;

	to->ae_en	 = from->AE_En;

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"\nW = %d, H = %d , BW = %d, BH = %d , X_S = %d, Y_S = %d , AE_EN = %d\n",
		to->grid_width,
		to->grid_height,
		to->block_width,
		to->block_height,
		to->x_start,
		to->y_start,
		to->ae_en);

	IA_CSS_LEAVE_PRIVATE("");
}

static void ia_css_3a_grid_config_ddr_decode(struct ia_css_2500_4a_config *to,
        struct ia_css_4a_private_config *from)
{
	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"ia_css_3a_grid_config_ddr_decode() enter\n");

	ia_css_awb_grid_config_ddr_decode(&to->awb_grd_config,
					  &from->awb_config);

	ia_css_af_grid_config_ddr_decode(&to->af_grd_config, &from->af_config);

	ia_css_awb_fr_grid_config_ddr_decode(&to->awb_fr_grd_config,
					     &from->awb_fr_config);

	ia_css_ae_grid_config_ddr_decode(&to->ae_grd_config,
					 &from->ae_grd_config);

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"ia_css_3a_grid_config_ddr_decode() leave\n");
}

/*
  awb_debubble -
  Removes bubbles btwn sets of stats caused by ACC
  Due to striping support set size might differ
  btwn the stripes  but will stay consistent...
  example:
  statistics layout before of ia_css_3a_debubble:
	  | stats ... 0000 ... stats .... 000 ....|
  statistics layout after of ia_css_3a_debubble:
	   | stats ... stats .... stats ...|
*/
static void awb_debubble(awb_public_raw_buffer_t *awb_raw_buffer,
                         struct bubble_info *awb_bubble_info)
{
    unsigned int num_sets, src_index, dst_index;
    unsigned int num_of_stripes;
    unsigned int i, set_size_w_bubble;

    IA_CSS_ENTER_PRIVATE("buffer=%p, bubble info=%p",
                         awb_raw_buffer, awb_bubble_info);


    num_of_stripes = awb_bubble_info[0].num_of_stripes;
    if (num_of_stripes <= 1)	{
        awb_bubble_info[1] = awb_bubble_info[0];
    }

    for(i = 0; i <  RES_MGR_PRIVATE_MAX_NUM_OF_STRIPES; i++)
    {
        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "bubble info stripe %d:\n",i);

        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "num_of_stirpes = %d \n",
                            awb_bubble_info[0].num_of_stripes);

        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "num_sets = %d \n",
                            awb_bubble_info[i].num_sets);

        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "size_of_set = %d \n",
                            awb_bubble_info[i].size_of_set);

        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "bubble_size = %d \n",
                            awb_bubble_info[i].bubble_size);
    }

    if (awb_bubble_info[0].bubble_size || awb_bubble_info[1].bubble_size)
    {

        dst_index = awb_bubble_info[0].size_of_set;

        set_size_w_bubble = awb_bubble_info[0].size_of_set +
                            awb_bubble_info[0].bubble_size;

        src_index = set_size_w_bubble;

        /* number of sets for s0 and s1 are identical */
        if (num_of_stripes <= 1)
            num_sets =  awb_bubble_info[0].num_sets;
        else {
            if (awb_bubble_info[0].bubble_size) {
                num_sets = awb_bubble_info[0].num_sets*2;
            }
            else {
                num_sets = awb_bubble_info[1].num_sets*2;
            }
        }
        for (i = 1; i < num_sets; i++)
        {
            memmove((void *)&awb_raw_buffer->rgb_table[dst_index],
                    (void *)&awb_raw_buffer->rgb_table[src_index],
                    (sizeof(awb_public_set_item_t) * awb_bubble_info[i%2].size_of_set));

            set_size_w_bubble = awb_bubble_info[i%2].size_of_set +
                                awb_bubble_info[i%2].bubble_size;

            src_index += set_size_w_bubble;
            dst_index +=  awb_bubble_info[i%2].size_of_set;
        }
    }

    IA_CSS_LEAVE_PRIVATE("");

}

/*
  af_debubble-
  Removes bubbles btwn sets of stats caused by ACC
  Due to striping support set size might differ
  btwn the stripes  but will stay consistent...
  example:
  statistics layout before of ia_css_3a_debubble:
	 | stats ... 0000 ... stats .... 000 ....|
  statistics layout after of ia_css_3a_debubble:
	 | stats ... stats .... stats ...|
*/
static void ia_css_af_debubble(af_public_raw_buffer_t *af_raw_buffer,
                               struct bubble_info *af_bubble_info)
{
    unsigned int num_sets, src_index, dst_index;
    unsigned int num_of_stripes = 0;
    unsigned int i, set_size_w_bubble;

    IA_CSS_ENTER_PRIVATE("buffer=%p, bubble info=%p",
                         af_raw_buffer, af_bubble_info);

    num_of_stripes = af_bubble_info[0].num_of_stripes;
    if (num_of_stripes <= 1)	{
        af_bubble_info[1] =  af_bubble_info[0];
    }

    for(i = 0; i <  RES_MGR_PRIVATE_MAX_NUM_OF_STRIPES; i++)
    {
        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "bubble info stripe %d:\n",i);

        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "num_of_stirpes = %d\n ",
                            af_bubble_info[0].num_of_stripes);

        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "num_sets = %d \n",
                            af_bubble_info[i].num_sets);

        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "size_of_set = %d \n",
                            af_bubble_info[i].size_of_set);

        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "bubble_size = %d \n",
                            af_bubble_info[i].bubble_size);
    }

    if (af_bubble_info[0].bubble_size || af_bubble_info[1].bubble_size)
    {

        dst_index = af_bubble_info[0].size_of_set;

        set_size_w_bubble = af_bubble_info[0].size_of_set +
                            af_bubble_info[0].bubble_size;

        src_index = set_size_w_bubble;

        /* number of sets for s0 and s1 are identical */
        if (num_of_stripes <= 1)
            num_sets =  af_bubble_info[0].num_sets;
        else {
            if (af_bubble_info[0].bubble_size) {
                num_sets = af_bubble_info[0].num_sets*2;
            }
            else {
                num_sets = af_bubble_info[1].num_sets*2;
            }
        }

        for (i = 1; i < num_sets; i++)
        {
            memmove((void *)&af_raw_buffer->y_table[dst_index],
                    (void *)&af_raw_buffer->y_table[src_index],
                    (sizeof(af_public_y_item_t) * af_bubble_info[i%2].size_of_set));

            set_size_w_bubble = af_bubble_info[i%2].size_of_set +
                                af_bubble_info[i%2].bubble_size;

            src_index += set_size_w_bubble;
            dst_index +=  af_bubble_info[i%2].size_of_set;
        }
    }

    IA_CSS_LEAVE_PRIVATE("");

}

/* awb_fr_debubble-
 * Removes bubbles btwn sets of stats caused by ACC
 * Due to striping support set size might differ
 * btwn the stripes  but will stay consistent...
 * example:
 * statistics layout before of ia_css_3a_debubble:
 *  | stats ... 0000 ... stats .... 000 ....|
 * statistics layout after of ia_css_3a_debubble:
 *  | stats ... stats .... stats ...|
 */

static void ia_css_awb_fr_debubble(awb_fr_public_raw_buffer_t *awb_fr_raw_buffer,
                                   struct bubble_info *awb_fr_bubble_info)
{
    unsigned int num_sets, src_index,dst_index;
    unsigned int num_of_stripes = 0;
    unsigned int i, set_size_w_bubble;

    IA_CSS_ENTER_PRIVATE("buffer=%p, bubble info=%p",
                         awb_fr_raw_buffer, awb_fr_bubble_info);

    num_of_stripes = awb_fr_bubble_info[0].num_of_stripes;
    if (num_of_stripes <= 1) {
        awb_fr_bubble_info[1] =  awb_fr_bubble_info[0];
    }

    for(i = 0; i < RES_MGR_PRIVATE_MAX_NUM_OF_STRIPES; i++)
    {
        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "bubble info stripe %d:\n",i);

        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "num_of_stirpes = %d\n",
                            awb_fr_bubble_info[0].num_of_stripes);

        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "num_sets = %d \n",
                            awb_fr_bubble_info[i].num_sets);

        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "size_of_set = %d \n",
                            awb_fr_bubble_info[i].size_of_set);

        ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                            "bubble_size = %d \n",
                            awb_fr_bubble_info[i].bubble_size);
    }

    if (awb_fr_bubble_info[0].bubble_size || awb_fr_bubble_info[1].bubble_size)
    {

        dst_index = awb_fr_bubble_info[0].size_of_set;

        set_size_w_bubble = awb_fr_bubble_info[0].size_of_set +
                            awb_fr_bubble_info[0].bubble_size;

        src_index = set_size_w_bubble;

        // number of sets for s0 and s1 are identical
        if (num_of_stripes <= 1)
            num_sets =  awb_fr_bubble_info[0].num_sets;
        else {
            if (awb_fr_bubble_info[0].bubble_size) {
                num_sets = awb_fr_bubble_info[0].num_sets*2;
            }
            else {
                num_sets = awb_fr_bubble_info[1].num_sets*2;
            }
        }

        for(i=1; i < num_sets; i++)
        {
            memmove((void *)&awb_fr_raw_buffer->bayer_table[dst_index],
                    (void *)&awb_fr_raw_buffer->bayer_table[src_index],
                    (sizeof(awb_fr_public_bayer_item_t) * awb_fr_bubble_info[i%2].size_of_set));

            set_size_w_bubble = awb_fr_bubble_info[i%2].size_of_set +
                                awb_fr_bubble_info[i%2].bubble_size;

            src_index += set_size_w_bubble;
            dst_index +=  awb_fr_bubble_info[i%2].size_of_set;
        }
    }

    IA_CSS_LEAVE_PRIVATE("");

}

/*
  ia_css_3a_debubble -
  Removes bubbles btwn sets of stats caused by ACC
  Due to striping support set size might differ
  btwn the stripes  but will stay consistent...
  example:
  statistics layout before of ia_css_3a_debubble:
	  | stats ... 0000 ... stats .... 000 ....|
  statistics layout after of ia_css_3a_debubble:
	   | stats ... stats .... stats ...|
*/
static void ia_css_3a_debubble(struct stats_4a_public_raw_buffer *meta_data,
                               struct stats_3a_bubble_info_per_stripe *bubble_info,
                               struct ff_status *stats_enable)
{
    ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                        "ia_css_3a_debubble() enter\n");

    if(stats_enable->awb_en)
    {
        awb_debubble(&meta_data->awb_raw_buffer,
                     bubble_info->awb_bubble_info);
    }
    if(stats_enable->af_en)
    {
        ia_css_af_debubble(&meta_data->af_raw_buffer,
                           bubble_info->af_bubble_info);
    }
    if(stats_enable->awb_fr_en)
    {
        ia_css_awb_fr_debubble(&meta_data->awb_fr_raw_buffer,
                               bubble_info->awb_fr_bubble_info);
    }
    ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
                        "ia_css_3a_debubble() leave\n");
}

void IPAIPU3Stats::ipu3_stats_init_3a(struct ipu3_stats_all_stats *all_stats)
{
	all_stats->ia_css_4a_statistics.data =
		&all_stats->stats_4a_public_raw_buffer;
	all_stats->ia_css_4a_statistics.stats_4a_config =
		&all_stats->ia_css_2500_4a_config;
}

void
IPAIPU3Stats::ipu3_stats_get_3a([[maybe_unused]] struct ipu3_stats_all_stats *all_stats,
				[[maybe_unused]] const struct ipu3_uapi_stats_3a *isp_stats)
{
	/* extract, memcpy and debubble each of 3A stats */
	struct ia_css_4a_statistics *host_stats = &all_stats->ia_css_4a_statistics;
	struct ia_css_4a_private_config stats_config;
	struct stats_3a_bubble_info_per_stripe stats_bubble_info;
	struct ff_status stats_enable;
	ae_private_raw_buffer_aligned_t ae_raw_buffer_s;
	unsigned int ae_join_buffers;

	hrt_vaddress af_ddr_addr = (hrt_vaddress)(long int)
				   &(((struct stats_4a_private_raw_buffer *)(long int)isp_stats)->af_raw_buffer);

	hrt_vaddress awb_ddr_addr = (hrt_vaddress)(long int)
				    &((struct stats_4a_private_raw_buffer *)(long int)isp_stats)->awb_raw_buffer;

	hrt_vaddress awb_fr_ddr_addr = (hrt_vaddress)(long int)
				        &((struct stats_4a_private_raw_buffer *)(long int)isp_stats)->awb_fr_raw_buffer;

	hrt_vaddress ae_buff_0_ddr_addr = (hrt_vaddress)(long int)
					  &((struct stats_4a_private_raw_buffer *)(long int)isp_stats)->ae_raw_buffer[0];
	hrt_vaddress ae_buff_1_ddr_addr = (hrt_vaddress)(long int)
					  &((struct stats_4a_private_raw_buffer *)(long int)isp_stats)->ae_raw_buffer[1];

	hrt_vaddress ae_pp_info_addr = (hrt_vaddress)(long int)
					&((struct stats_4a_private_raw_buffer *)(long int)isp_stats)->ae_join_buffers;

	hrt_vaddress stats_bubble_info_addr = (hrt_vaddress) &(isp_stats->stats_3a_bubble_per_stripe);

	hrt_vaddress stats_config_addr = (hrt_vaddress) &(isp_stats->stats_4a_config);

	hrt_vaddress stats_3a_enable = (hrt_vaddress)(long int)
					&((struct stats_4a_private_raw_buffer *)(long int)isp_stats)->stats_3a_status;

	ipu3_stats_init_3a(all_stats);

	/* load grid configuration */
	mmgr_load(stats_config_addr,
		 (void *)&(stats_config),
		 sizeof(struct  ia_css_4a_private_config));

	/* load bubble info */
	mmgr_load(stats_bubble_info_addr, (void *)&(stats_bubble_info),
		  sizeof(struct stats_3a_bubble_info_per_stripe));

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

	mmgr_load(stats_3a_enable,
		 (void *)&(stats_enable),
		  sizeof(struct ff_status));

	if (ae_join_buffers == 1) {
		mmgr_load(ae_buff_1_ddr_addr,
			 (void *)&(ae_raw_buffer_s),
			 sizeof(ae_private_raw_buffer_aligned_t));
	}

	/* decode must be prior to debubbling! */
	ia_css_3a_grid_config_ddr_decode(host_stats->stats_4a_config,
					 &stats_config);

	/* for striping might need to combine buffers of ae */
	if (ae_join_buffers == 1)
		ia_css_3a_join_ae_buffers(&host_stats->data->ae_raw_buffer,
					  &ae_raw_buffer_s);

	ia_css_3a_debubble(host_stats->data, &stats_bubble_info, &stats_enable);
}

ia_err
IPAIPU3Stats::intel_skycam_statistics_convert(const ia_css_4a_statistics& statistics,
					      ia_aiq_rgbs_grid* out_rgbs_grid,
					      ia_aiq_af_grid* out_af_grid)
{
    if (!out_rgbs_grid || !out_af_grid) {
        return ia_err_data;
    }

    // AWB (RGBS) grid.
    out_rgbs_grid->grid_width = statistics.stats_4a_config->awb_grd_config.grid_width;
    out_rgbs_grid->grid_height = statistics.stats_4a_config->awb_grd_config.grid_height;

    for (int i = 0; i < out_rgbs_grid->grid_width*out_rgbs_grid->grid_height; ++i) {
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

    for(int i = 0; i < out_af_grid->grid_width * out_af_grid->grid_height; ++i) {
        out_af_grid->filter_response_1[i] = statistics.data->af_raw_buffer.y_table[i].y1_avg;
        out_af_grid->filter_response_2[i] = statistics.data->af_raw_buffer.y_table[i].y2_avg;
    }

    return ia_err_none;
}

}

