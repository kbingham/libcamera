/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2017 Intel Corporation.
 *
 * IPAIPU3Stats.cpp: Generate statistics in IA AIQ consumable format.
 */

#include "ipa_ipu3_stats.h"

#include "libcamera/internal/log.h"

#include "ipu3_all_stats.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(IPAIPU3Stats)

IPAIPU3Stats::IPAIPU3Stats()
{
	aiqStatsInputParams_ = {};

	/* \todo: Is this fine here or we need separate helper? */
	rgbsGridBuffPool_ = std::make_shared<SharedItemPool<ia_aiq_rgbs_grid>>("RgbsGridBuffPool");
	afFilterBuffPool_ = std::make_shared<SharedItemPool<ia_aiq_af_grid>>("AfFilterBuffPool");
#define PUBLIC_STATS_POOL_SIZE 9 /* comes from CrOS */
	int ret = allocateStatBufferPools(PUBLIC_STATS_POOL_SIZE);
	if (ret < 0)
		LOG(IPAIPU3Stats, Error) << "Failed to allocate stats grid buffers";
}

IPAIPU3Stats::~IPAIPU3Stats()
{
	freeStatBufferPools();
	rgbsGridBuffPool_.reset();
	afFilterBuffPool_.reset();
}

ia_aiq_statistics_input_params *
IPAIPU3Stats::getInputStatsParams(int frame, ipa::ipu3::aiq::AiqResults *results,
				  const ipu3_uapi_stats_3a *stats)
{
	aiqStatsInputParams_.frame_id = frame;
	aiqStatsInputParams_.frame_ae_parameters = results->ae();
	aiqStatsInputParams_.frame_af_parameters = results->af();
	aiqStatsInputParams_.awb_results = results->awb();
	aiqStatsInputParams_.frame_pa_parameters = results->pa();
	aiqStatsInputParams_.frame_sa_parameters = results->sa();
	aiqStatsInputParams_.camera_orientation = ia_aiq_camera_orientation_unknown;

	IPU3AllStats::ipu3_stats_all_stats outStats;
	memset(&outStats, 0, sizeof(IPU3AllStats::ipu3_stats_all_stats));
	IPU3AllStats::ipu3_stats_get_3a(&outStats, stats);

	std::shared_ptr<ia_aiq_rgbs_grid> rgbsGrid = nullptr;
	std::shared_ptr<ia_aiq_af_grid> afGrid = nullptr;
	int ret = afFilterBuffPool_->acquireItem(afGrid);
	ret |= rgbsGridBuffPool_->acquireItem(rgbsGrid);
	if (ret != 0 || afGrid.get() == nullptr || rgbsGrid.get() == nullptr) {
		LOG(IPAIPU3Stats, Error) << "Failed to acquire 3A buffers from pools";
		return nullptr;
	}

	IPU3AllStats::intel_skycam_statistics_convert(outStats.ia_css_4a_statistics,
						      rgbsGrid.get(), afGrid.get());

	const ia_aiq_rgbs_grid *rgbsGridPtr = rgbsGrid.get();
	const ia_aiq_af_grid *afGridPtr = afGrid.get();

	aiqStatsInputParams_.num_rgbs_grids = 1;
	aiqStatsInputParams_.rgbs_grids = &rgbsGridPtr;
	aiqStatsInputParams_.num_af_grids = 1;
	aiqStatsInputParams_.af_grids = &afGridPtr;

	return &aiqStatsInputParams_;
}

int IPAIPU3Stats::allocateStatBufferPools(int numBufs)
{
	int ret = afFilterBuffPool_->init(numBufs);
	ret |= rgbsGridBuffPool_->init(numBufs);
	if (ret != 0) {
		LOG(IPAIPU3Stats, Error) << "Failed to initialize 3A statistics pools";
		freeStatBufferPools();
		return -ENOMEM;
	}
#define IPU3_MAX_STATISTICS_WIDTH 80
#define IPU3_MAX_STATISTICS_HEIGHT 60
	int maxGridSize = IPU3_MAX_STATISTICS_WIDTH * IPU3_MAX_STATISTICS_HEIGHT;
	std::shared_ptr<ia_aiq_rgbs_grid> rgbsGrid = nullptr;
	std::shared_ptr<ia_aiq_af_grid> afGrid = nullptr;

	for (int allocated = 0; allocated < numBufs; allocated++) {
		ret = afFilterBuffPool_->acquireItem(afGrid);
		ret |= rgbsGridBuffPool_->acquireItem(rgbsGrid);

		if (ret != 0 || afGrid.get() == nullptr ||
		    rgbsGrid.get() == nullptr) {
			LOG(IPAIPU3Stats, Error) << "Failed to acquire memory from pools";
			freeStatBufferPools();
			return -ENOMEM;
		}

		rgbsGrid->blocks_ptr = new rgbs_grid_block[maxGridSize];
		rgbsGrid->grid_height = 0;
		rgbsGrid->grid_width = 0;

		afGrid->filter_response_1 = new int[maxGridSize];
		afGrid->filter_response_2 = new int[maxGridSize];
		afGrid->block_height = 0;
		afGrid->block_width = 0;
		afGrid->grid_height = 0;
		afGrid->grid_width = 0;
	}

	return 0;
}

void IPAIPU3Stats::freeStatBufferPools()
{
	if (!afFilterBuffPool_->isFull() || !rgbsGridBuffPool_->isFull()) {
		/* We will leak if we errored out in allocateStatBufferPools*/
		if (!afFilterBuffPool_->isFull())
			LOG(IPAIPU3Stats, Warning) << "AfFilterBuffPool is leaking";
		if (!rgbsGridBuffPool_->isFull())
			LOG(IPAIPU3Stats, Warning) << "RgbsGridBuffPool is leaking";
	}

	int ret = -1;
	size_t availableItems = afFilterBuffPool_->availableItems();
	std::shared_ptr<ia_aiq_af_grid> afGrid = nullptr;
	for (size_t i = 0; i < availableItems; i++) {
		ret = afFilterBuffPool_->acquireItem(afGrid);
		if (ret == 0 && afGrid.get() != nullptr) {
			delete[] afGrid->filter_response_1;
			afGrid->filter_response_1 = nullptr;
			delete[] afGrid->filter_response_2;
			afGrid->filter_response_2 = nullptr;
		} else {
			LOG(IPAIPU3Stats, Warning)
				<< "Could not acquire AF filter response "
				<< i << "for deletion - leak?";
		}
	}

	ret = -1;
	availableItems = rgbsGridBuffPool_->availableItems();
	std::shared_ptr<ia_aiq_rgbs_grid> rgbsGrid = nullptr;
	for (size_t i = 0; i < availableItems; i++) {
		ret = rgbsGridBuffPool_->acquireItem(rgbsGrid);
		if (ret == 0 && rgbsGrid.get() != nullptr) {
			delete[] rgbsGrid->blocks_ptr;
			rgbsGrid->blocks_ptr = nullptr;
		} else {
			LOG(IPAIPU3Stats, Warning)
				<< "Could not acquire RGBS grid " << i
				<< "for deletion - leak?";
		}
	}
}

} /* namespace libcamera */
