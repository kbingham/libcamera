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

#ifndef IPA_IPU3_STATS_H
#define IPA_IPU3_STATS_H

#include <ia_imaging/ia_aiq_types.h>
#include <ia_imaging/stats_3a_public.h>

#include <linux/intel-ipu3.h>

namespace libcamera {

class IPAIPU3Stats {
public:
	struct ipu3_stats_all_stats {
		struct ia_css_4a_statistics ia_css_4a_statistics;
		struct stats_4a_public_raw_buffer stats_4a_public_raw_buffer;
		struct ia_css_2500_4a_config ia_css_2500_4a_config;
	};

	void ipu3_stats_get_3a(struct ipu3_stats_all_stats *all_stats,
			       const struct ipu3_uapi_stats_3a *isp_stats);
	ia_err intel_skycam_statistics_convert(const ia_css_4a_statistics& statistics,
					       ia_aiq_rgbs_grid* out_rgbs_grid,
					       ia_aiq_af_grid* out_af_grid);

private:
	void ipu3_stats_init_3a(struct ipu3_stats_all_stats *all_stats);
};

} /* namespace libcamera */

#endif /* IPA_IPU3_STATS_H */
