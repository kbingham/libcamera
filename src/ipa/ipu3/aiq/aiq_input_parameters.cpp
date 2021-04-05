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

int AiqInputParameters::configureSensorParams(const CameraSensorInfo &sensorInfo)
{
	sensorDescriptor.pixel_clock_freq_mhz = sensorInfo.pixelRate / 1000000;
	sensorDescriptor.pixel_periods_per_line = sensorInfo.lineLength;
	sensorDescriptor.line_periods_per_field = sensorInfo.minFrameLength;
	sensorDescriptor.line_periods_vertical_blanking = 106; /* default */
	//INFO: fine integration is not supported by v4l2
	sensorDescriptor.fine_integration_time_min = 0;
	sensorDescriptor.fine_integration_time_max_margin = sensorDescriptor.pixel_periods_per_line;
	sensorDescriptor.coarse_integration_time_min = 4; /* min VBLANK */
	/* Guess from hal-configs-nautilus/files/camera3_profiles.xml#263 */
	sensorDescriptor.coarse_integration_time_max_margin = 10;

	return 0;
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

void AiqInputParameters::setAeAwbAfDefaults()
{
	/*Ae Params */
	aeInputParams.num_exposures = NUM_EXPOSURES;
	aeInputParams.frame_use = ia_aiq_frame_use_still;
	aeInputParams.flash_mode = ia_aiq_flash_mode_off;
	aeInputParams.operation_mode = ia_aiq_ae_operation_mode_automatic;
	aeInputParams.metering_mode = ia_aiq_ae_metering_mode_evaluative;
	aeInputParams.priority_mode = ia_aiq_ae_priority_mode_normal;
	aeInputParams.flicker_reduction_mode = ia_aiq_ae_flicker_reduction_off;
	aeInputParams.exposure_window = nullptr;
	aeInputParams.exposure_coordinate = nullptr;
	aeInputParams.ev_shift = 0;
	aeInputParams.sensor_descriptor = &sensorDescriptor;
	aeInputParams.manual_exposure_time_us = nullptr;
	aeInputParams.manual_analog_gain = nullptr;
	aeInputParams.manual_iso = nullptr;
	aeInputParams.aec_features = nullptr;
	aeInputParams.manual_limits = nullptr;
	aeInputParams.manual_aperture_fn = -1;
	aeInputParams.manual_dc_iris_command = ia_aiq_aperture_control_dc_iris_auto;
	aeInputParams.exposure_distribution_priority = ia_aiq_ae_exposure_distribution_shutter;
	aeInputParams.manual_convergence_time = -1;

	/* AWB Params */
	awbParams.frame_use = ia_aiq_frame_use_still;
	awbParams.scene_mode = ia_aiq_awb_operation_mode_auto;
	awbParams.manual_convergence_time = -1.0;
	awbParams.manual_cct_range = nullptr;
	awbParams.manual_white_coordinate = nullptr;

	/* AF Params */
	afParams = {
		ia_aiq_frame_use_still, 0, 1500,
		ia_aiq_af_operation_mode_auto,
		ia_aiq_af_range_normal,
		ia_aiq_af_metering_mode_auto,
		ia_aiq_flash_mode_off,
		NULL, NULL, false
	};

	/* GBCE Params */
	gbceParams.gbce_level = ia_aiq_gbce_level_gamma_stretch;
	gbceParams.tone_map_level = ia_aiq_tone_map_level_default;
	gbceParams.frame_use = ia_aiq_frame_use_still;
	gbceParams.ev_shift = 0;
}

} /* namespace aiq */
} /* namespace ipu3 */
} /* namespace ipa */

} /* namespace libcamera */
