/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2017 Intel Corporation.
 *
 * IPAIPU3Stats.cpp: Generate statistics in IA AIQ consumable format.
 */

#include "aiq/aiq.h"
#include "aiq/aiq_results.h"

#ifndef IPA_IPU3_STATS_H
#define IPA_IPU3_STATS_H

namespace libcamera {

struct AiqResults;

class IPAIPU3Stats
{
public:
	IPAIPU3Stats();

	ia_aiq_statistics_input_params *
	getInputStatsParams(int frame,
			    ipa::ipu3::aiq::AiqResults *results,
			    const ipu3_uapi_stats_3a *stats);

private:
	ia_aiq_statistics_input_params aiqStatsInputParams_;
};

} /* namespace libcamera */

#endif /* IPA_IPU3_STATS_H */
