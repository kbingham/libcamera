/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * aiq_input_parameters.cpp - Intel IA Imaging library C++ wrapper
 *
 * AIQ Input Parameters container, manages the parameters and state for each
 * algorithm.
 */

#include "aiq/aiq_input_parameters.h"

#include <algorithm>

#include "libcamera/internal/log.h"

/* Macros used by imported code */
#define STDCOPY(dst, src, size) std::copy((src), ((src) + (size)), (dst))
#define MEMCPY_S(dest, dmax, src, smax) memcpy((dest), (src), std::min((size_t)(dmax), (size_t)(smax)))
#define CLEAR(x) memset(&(x), 0, sizeof(x))

namespace libcamera {

LOG_DEFINE_CATEGORY(AIQInputParameters)

namespace ipa {
namespace ipu3 {
namespace aiq {

void AiqInputParameters::init()
{
	CLEAR(aeInputParams);
	CLEAR(afParams);
	CLEAR(afBracketParams);
	CLEAR(awbParams);
	CLEAR(gbceParams);
	CLEAR(paParams);
	CLEAR(saParams);
	CLEAR(sensorDescriptor);
	CLEAR(exposureWindow);
	CLEAR(exposureCoordinate);
	CLEAR(aeFeatures);
	CLEAR(aeManualLimits);
	CLEAR(manualFocusParams);
	CLEAR(focusRect);
	CLEAR(manualCctRange);
	CLEAR(manualWhiteCoordinate);
	CLEAR(awbResults);
	CLEAR(colorGains);
	CLEAR(exposureParams);
	CLEAR(sensorFrameParams);
	aeLock = false;
	awbLock = false;
	blackLevelLock = false;
	/* \todo: afRegion.reset(); */

	reset();
}

void AiqInputParameters::reset()
{
	aeInputParams.sensor_descriptor = &sensorDescriptor;
	aeInputParams.exposure_window = &exposureWindow;
	aeInputParams.exposure_coordinate = &exposureCoordinate;
	aeInputParams.aec_features = &aeFeatures;
	aeInputParams.manual_limits = &aeManualLimits;
	aeInputParams.manual_exposure_time_us = &manual_exposure_time_us[0];
	aeInputParams.manual_analog_gain = &manual_analog_gain[0];
	aeInputParams.manual_iso = &manual_iso[0];
	aeInputParams.manual_convergence_time = -1;

	afParams.focus_rect = &focusRect;
	afParams.manual_focus_parameters = &manualFocusParams;

	awbParams.manual_cct_range = &manualCctRange;
	awbParams.manual_white_coordinate = &manualWhiteCoordinate;

	paParams.awb_results = &awbResults;
	paParams.color_gains = &colorGains;
	paParams.exposure_params = &exposureParams;

	saParams.awb_results = &awbResults;
	saParams.sensor_frame_params = &sensorFrameParams;
}

AiqInputParameters &AiqInputParameters::operator=(const AiqInputParameters &other)
{
	if (this == &other)
		return *this;

	MEMCPY_S(this,
		 sizeof(AiqInputParameters),
		 &other,
		 sizeof(AiqInputParameters));
	reset();

	/* Exposure coordinate is nullptr in other than SPOT mode. */
	if (other.aeInputParams.exposure_coordinate == nullptr)
		aeInputParams.exposure_coordinate = nullptr;

	/* focus_rect and manual_focus_parameters may be nullptr */
	if (other.afParams.focus_rect == nullptr)
		afParams.focus_rect = nullptr;
	if (other.afParams.manual_focus_parameters == nullptr)
		afParams.manual_focus_parameters = nullptr;

	/* manual_cct_range and manual_white_coordinate may be nullptr */
	if (other.awbParams.manual_cct_range == nullptr)
		awbParams.manual_cct_range = nullptr;
	if (other.awbParams.manual_white_coordinate == nullptr)
		awbParams.manual_white_coordinate = nullptr;

	return *this;
}

} /* namespace aiq */
} /* namespace ipu3 */
} /* namespace ipa */

} /* namespace libcamera */
