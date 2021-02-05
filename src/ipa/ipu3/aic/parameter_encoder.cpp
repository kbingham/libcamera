/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2017 Intel Corporation.
 *
 * ParameterEncoder.cpp: Encode AIC parameters to IPU3 kernel interface
 *
 * This implementation is highly derived from ChromeOS:
 *   platform2/camera/hal/intel/ipu3/psl/ipu3/workers/IPU3AicToFwEncoder.cpp
 */

#include "parameter_encoder.h"

namespace libcamera {

void ParameterEncoder::encode(aic_config *config, ipu3_uapi_params *params)
{
	/*
	 * Do not encode parameters until the algorithms are run,
	 * or assertions will be hit
	 */
	return;

	(void)config;
	(void)params;

	return;
}

} /* namespace libcamera */
