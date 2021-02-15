/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Ideas On Board
 *
 * ipu3_awb.h - IPU3 AWB control algorithm
 */
#ifndef __LIBCAMERA_IPU3_AWB_H__
#define __LIBCAMERA_IPU3_AWB_H__

#include <linux/intel-ipu3.h>

#include <libcamera/geometry.h>

#include "libipa/algorithm.h"

namespace libcamera {

namespace ipa {

class IPU3Awb : public Algorithm
{
public:
	IPU3Awb();
	~IPU3Awb();

	void initialise(ipu3_uapi_params &params, const Size &bdsOutputSize);
	void calculateWBGains(Rectangle roi,
			      const ipu3_uapi_stats_3a *stats);
	void updateWbParameters(ipu3_uapi_params &params, double agcGamma);

private:
	uint32_t estimateCCT(uint8_t red, uint8_t green, uint8_t blue);

	/* WB calculated gains */
	uint16_t wbGains_[4];
	uint32_t cct_ = 0;

	uint32_t frame_count_;
	struct ipu3_uapi_grid_config bdsGrid_;
};

} /* namespace ipa */

} /* namespace libcamera*/
#endif /* __LIBCAMERA_IPU3_AWB_H__ */
