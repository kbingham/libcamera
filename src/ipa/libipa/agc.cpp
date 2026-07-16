/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2026 Ideas On Board
 *
 * libIPA Agc algorithm
 */

#include "agc.h"

/**
 * \file agc.h
 * \brief libipa AGC algorithm
 */

namespace libcamera {

namespace ipa {

namespace agc {

/**
 * \fn extractControls(const ControlList &controls, const CameraSensorHelper *sensor)
 * \param[in] controls The controls list to extract from
 * \param[in] sensor The CameraSensorHelper
 *
 * This function extracts \a V4L2_CID_EXPOSURE and \a V4L2_CID_ANALOGUE_GAIN
 * from \a controls and then returns the exposure and gain values. The gain
 * code is mapped to the real gain value if \a sensor is provided, otherwise
 * the gain code is returned.
 *
 * \return A pair of exposure and analogue gain extracted from \a controls
 */

/**
 * \fn prepareControls(ControlList &controls, const CameraSensorHelper *sensor,
 *                     uint32_t exposure, double gain)
 * \param[out] controls The controls list to populate
 * \param[in] sensor The CameraSensorHelper
 * \param[in] exposure The exposure (in lines)
 * \param[in] gain The analogue gain
 *
 * This function sets \a V4L2_CID_EXPOSURE and \a V4L2_CID_ANALOGUE_GAIN
 * in \a controls. The gain is mapped to the gain code if \a sensor is provided,
 * otherwise the gain value will be used directly.
 */

} /* namespace agc */

} /* namespace ipa */

} /* namespace libcamera */
