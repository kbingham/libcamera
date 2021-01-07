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

#ifndef _STATS_3A_PUBLIC_H_
#define _STATS_3A_PUBLIC_H_

/** @file
* CSS-API header file for 2500/Skycam 3A statistics datastructures.
*/

#include <awb_public.h>
#include <af_public.h>
#include <ae_public.h>
#include <awb_fr_public.h>
typedef enum{
	MAX_SIZE_OF_SET_AF=32,
	MAX_SIZE_OF_SET_AWB_FR=32,
	MAX_SIZE_OF_SET_AWB=160
}stat_max_size_of_set ;

typedef enum {
  stat_af,
  stat_awb_fr,
  stat_ae,
  stat_awb
} statistic_type_t;

struct ia_css_2500_4a_config {
	struct ae_public_config_grid_config			ae_grd_config;
	struct awb_public_config_grid_config		awb_grd_config;
	struct af_public_grid_config				af_grd_config;
	struct awb_fr_public_grid_config			awb_fr_grd_config;
};

 struct stats_4a_public_raw_buffer {
   awb_public_raw_buffer_t awb_raw_buffer;
   ae_public_raw_buffer_t  ae_raw_buffer;
   awb_fr_public_raw_buffer_t  awb_fr_raw_buffer;
   af_public_raw_buffer_t  af_raw_buffer;

};

 struct ia_css_4a_statistics {
 	struct ia_css_2500_4a_config * stats_4a_config;
 	struct stats_4a_public_raw_buffer *data;
};

/** use ia_css_s3a_roi_offset struct to set 3a ROI coordinates */
struct ia_css_s3a_roi_offset {
	unsigned short x_coord; /**< left coordinate */
	unsigned short y_coord; /**< top coordinate */
};

#endif /* _STATS_3A_PUBLIC_H_ */
