/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google
 *
 * grid.h - IPU3 grid configuration
 */
#ifndef __LIBCAMERA_IPU3_ALGORITHMS_GRID_H__
#define __LIBCAMERA_IPU3_ALGORITHMS_GRID_H__

#include "algorithm.h"

namespace libcamera {

namespace ipa::ipu3::algorithms {

class Grid : public Algorithm
{
public:
	~Grid() = default;

	int configure(IPAContext &context, const IPAConfigInfo &configInfo) override;
	void prepare(IPAContext &context, ipu3_uapi_params &params) override;
};

} /* namespace ipa::ipu3::algorithms */

} /* namespace libcamera */

#endif /* __LIBCAMERA_IPU3_ALGORITHMS_CONTRAST_H__ */

