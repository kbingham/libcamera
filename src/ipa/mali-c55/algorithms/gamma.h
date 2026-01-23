/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2026, Ideas On Board
 *
 * MaliC55 RGB Gamma control
 */

#pragma once

#include <libipa/fixedpoint.h>
#include <libipa/gamma.h>

#include "algorithm.h"

namespace libcamera {

namespace ipa::mali_c55::algorithms {

class RGBGamma : public Algorithm
{
public:
	int init(IPAContext &context, const ValueNode &tuningData) override;
	int configure(IPAContext &context,
		      const IPACameraSensorInfo &configInfo) override;
	void queueRequest(IPAContext &context,
			  const uint32_t frame,
			  IPAFrameContext &frameContext,
			  const ControlList &controls) override;
	void prepare(IPAContext &context, const uint32_t frame,
		     IPAFrameContext &frameContext,
		     MaliC55Params *params) override;
	void process(IPAContext &context, const uint32_t frame,
		     IPAFrameContext &frameContext,
		     const mali_c55_stats_buffer *stats,
		     ControlList &metadata) override;

private:
	GammaAlgorithm<129, UQ<0, 12>> gamma_;
};

} /* namespace ipa::mali_c55::algorithms */
} /* namespace libcamera */
