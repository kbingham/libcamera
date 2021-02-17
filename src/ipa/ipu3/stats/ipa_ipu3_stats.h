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

#include <ia_imaging/ia_aiq_types.h>

#include "shared_item_pool.h"

namespace libcamera {

struct AiqResults;

class IPAIPU3Stats
{
public:
	IPAIPU3Stats();
	~IPAIPU3Stats();

	ia_aiq_statistics_input_params *
	getInputStatsParams(int frame,
			    ipa::ipu3::aiq::AiqResults *results,
			    const ipu3_uapi_stats_3a *stats);

private:
	void freeStatBufferPools();
	int allocateStatBufferPools(int numBufs);

	ia_aiq_statistics_input_params aiqStatsInputParams_;
	std::shared_ptr<SharedItemPool<ia_aiq_af_grid>> afFilterBuffPool_;
	std::shared_ptr<SharedItemPool<ia_aiq_rgbs_grid>> rgbsGridBuffPool_;
};

} /* namespace libcamera */

#endif /* IPA_IPU3_STATS_H */

