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

namespace libcamera {

void IPU3AllStats::ipu3_stats_get_3a([[maybe_unused]] struct ipu3_stats_all_stats *all_stats,
				     [[maybe_unused]] const struct ipu3_uapi_stats_3a *isp_stats)
{
	/* extract, memcpy and debubble each of 3A stats */
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

