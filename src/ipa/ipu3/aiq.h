/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * aiq.h - Intel IA Imaging library C++ wrapper
 *
 * To simplify naming, and prevent confusion the wrapper is named simply aiq
 * rather than ia_aiq.
 *
 * Bikeshedding opportunity: We could also name this
 *	ia_aiq++
 *	ia_aiqpp
 *	ia_aiq_wrapper
 *	...
 */

#include <ia_imaging/ia_aiq.h>
#include <linux/intel-ipu3.h>

#include "aic.h"

namespace libcamera {

class AIQ
{
public:
	AIQ();
	~AIQ();

	int init();
	int configure();
	int setStatistics(unsigned int frame, const ipu3_uapi_stats_3a *stats);
	int run(unsigned int frame, ipu3_uapi_params *params);

private:
	int af_run();
	int gbce_run();
	int ae_run();
	int awb_run();

	ia_aiq *aiq_;
	std::string version_;

	/*
	 * Run state configuration
	 * TODO: This may need to a new instance per run.
	 */
	aic_config_t config_;
};

} /* namespace libcamera */
