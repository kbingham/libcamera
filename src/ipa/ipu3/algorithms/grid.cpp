/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Ideas On Board
 *
 * grid.cpp - IPU3 grid configuration
 */

#include "grid.h"

#include <libcamera/base/log.h>

namespace libcamera {

namespace ipa::ipu3::algorithms {

LOG_DEFINE_CATEGORY(IPU3Grid)

/* Maximum number of cells on a row */
static constexpr uint32_t kMaxCellWidthPerSet = 160;
/* Maximum number of cells on a column */
static constexpr uint32_t kMaxCellHeightPerSet = 56;

/**
 * This function calculates a grid for the AWB algorithm in the IPU3 firmware.
 * Its input is the BDS output size calculated in the ImgU.
 * It is limited for now to the simplest method: find the lesser error
 * with the width/height and respective log2 width/height of the cells.
 *
 * \todo The frame is divided into cells which can be 8x8 => 128x128.
 * As a smaller cell improves the algorithm precision, adapting the
 * x_start and y_start parameters of the grid would provoke a loss of
 * some pixels but would also result in more accurate algorithms.
 */
int Grid::configure(IPAContext &context, const IPAConfigInfo &configInfo)
{
	uint32_t minError = std::numeric_limits<uint32_t>::max();
	Size best;
	Size bestLog2;
	ipu3_uapi_grid_config &bdsGrid = context.configuration.grid.bdsGrid;

	context.configuration.grid.bdsOutputSize = configInfo.bdsOutputSize;
	Size &bdsOutputSize = context.configuration.grid.bdsOutputSize;

	bdsGrid.x_start = 0;
	bdsGrid.y_start = 0;

	for (uint32_t widthShift = 3; widthShift <= 7; ++widthShift) {
		uint32_t width = std::min(kMaxCellWidthPerSet,
					  bdsOutputSize.width >> widthShift);
		width = width << widthShift;
		for (uint32_t heightShift = 3; heightShift <= 7; ++heightShift) {
			int32_t height = std::min(kMaxCellHeightPerSet,
						  bdsOutputSize.height >> heightShift);
			height = height << heightShift;
			uint32_t error = std::abs(static_cast<int>(width - bdsOutputSize.width))
				       + std::abs(static_cast<int>(height - bdsOutputSize.height));

			if (error > minError)
				continue;

			minError = error;
			best.width = width;
			best.height = height;
			bestLog2.width = widthShift;
			bestLog2.height = heightShift;
		}
	}

	bdsGrid.width = best.width >> bestLog2.width;
	bdsGrid.block_width_log2 = bestLog2.width;
	bdsGrid.height = best.height >> bestLog2.height;
	bdsGrid.block_height_log2 = bestLog2.height;

	LOG(IPU3Grid, Debug) << "Best grid found is: ("
			     << (int)bdsGrid.width << " << " << (int)bdsGrid.block_width_log2 << ") x ("
			     << (int)bdsGrid.height << " << " << (int)bdsGrid.block_height_log2 << ")";

	return 0;
}

} /* namespace ipa::ipu3::algorithms */

} /* namespace libcamera */
