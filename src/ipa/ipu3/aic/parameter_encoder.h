/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2017 Intel Corporation.
 *
 * ParameterEncoder.h: Encode AIC parameters to IPU3 kernel interface
 *
 * This implementation is highly derived from ChromeOS:
 *   platform2/camera/hal/intel/ipu3/psl/ipu3/workers/IPU3AicToFwEncoder.h
 */

#include <ia_imaging/ia_aiq.h>
#include <linux/intel-ipu3.h>

/* We wrap the AIC common types ourselves due to excessive header trails */
#include "kbl_aic.h"

#ifndef IPA_IPU3_PARAMETER_ENCODER
#define IPA_IPU3_PARAMETER_ENCODER

namespace libcamera {

namespace ParameterEncoder {
void encode(aic_config *config, ipu3_uapi_params *params);
}

} /* namespace libcamera */

#endif /* IPA_IPU3_PARAMETER_ENCODER */
