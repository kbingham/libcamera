/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Red Hat Inc.
 *
 * Exposure and gain
 */

#pragma once

#include <libipa/agc_msv.h>

#include "algorithm.h"

namespace libcamera {

namespace ipa::softisp::algorithms {

class Agc : public Algorithm
{
public:
	int configure(IPAContext &context, const IPAConfigInfo &configInfo) override;

	void process(IPAContext &context, const uint32_t frame,
		     IPAFrameContext &frameContext,
		     const SwIspStats *stats,
		     ControlList &metadata) override;

private:
	AgcMSV agc_;
};

} /* namespace ipa::softisp::algorithms */

} /* namespace libcamera */
