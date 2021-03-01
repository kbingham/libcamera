/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2017 Intel Corporation.
 *
 * IPAIPU3Stats.cpp: Generate statistics in IA AIQ consumable format.
 */

#include "ipa_ipu3_stats.h"

#include "libcamera/internal/log.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(IPAIPU3Stats)

IPAIPU3Stats::IPAIPU3Stats()
{
	aiqStatsInputParams_ = {};
}

ia_aiq_statistics_input_params *
IPAIPU3Stats::getInputStatsParams(int frame, ipa::ipu3::aiq::AiqResults *results,
				  [[maybe_unused]] const ipu3_uapi_stats_3a *stats)
{
	aiqStatsInputParams_.frame_id = frame;
	aiqStatsInputParams_.frame_ae_parameters = results->ae();
	aiqStatsInputParams_.frame_af_parameters = results->af();
	aiqStatsInputParams_.awb_results = results->awb();
	aiqStatsInputParams_.frame_pa_parameters = results->pa();
	aiqStatsInputParams_.frame_sa_parameters = results->sa();
	aiqStatsInputParams_.camera_orientation = ia_aiq_camera_orientation_unknown;

	return &aiqStatsInputParams_;
}

} /* namespace libcamera */
