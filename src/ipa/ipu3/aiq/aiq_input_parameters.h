/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * aiq_input_parameters.h - Intel IA Imaging library C++ wrapper
 *
 * AIQ Input Parameters container, manages the parameters and state for each
 * algorithm.
 */

#include <ia_imaging/ia_aiq.h>

#include <libcamera/internal/camera_sensor.h>

#ifndef IPA_IPU3_AIQ_INPUT_PARAMETERS_H
#define IPA_IPU3_AIQ_INPUT_PARAMETERS_H

namespace libcamera::ipa::ipu3::aiq {

static const unsigned int NUM_EXPOSURES = 1; /*!> Number of frames AIQ algorithm
						  provides output for */

/**
 * \struct AiqInputParams
 * The private structs are part of AE, AF and AWB input parameters.
 * They need to separately be introduced to store the contents for
 * the corresponding pointers.
 */
struct AiqInputParameters {
	void init();
	void reset();
	int configureSensorParams(const CameraSensorInfo &sensorInfo);
	void setAeAwbAfDefaults();
	AiqInputParameters &operator=(const AiqInputParameters &other);

	ia_aiq_ae_input_params aeInputParams;
	ia_aiq_af_input_params afParams;
	ia_aiq_af_bracket_input_params afBracketParams;
	ia_aiq_awb_input_params awbParams;
	ia_aiq_gbce_input_params gbceParams;
	ia_aiq_pa_input_params paParams;
	ia_aiq_sa_input_params saParams;
	ia_aiq_dsd_input_params dsdParams;

	/**
	 * We do not directly parse the AF region in the settings to the
	 * afParams focus_rectangle.
	 * The fillAfInputParams will output the AF region in this member.
	 * The reason is that not all HW platforms will implement touch AF
	 * by passing the focus rectangle to the AF algo. The current implementation
	 * assume that AF will get AF statistics covering the whole image.
	 * This is not always true.
	 * Some platforms modify the statistic collection parameters instead. So
	 * by modifying from where we get the statistics we can also achieve the
	 * effect of touch focus.
	 * It will be up to the PSL implementation to make use of the afRegion.
	 */
	/* \todo: Pull in the CameraWindow class if required */
	//CameraWindow afRegion; /*!> AF region in IA_COORDINATE space parsed
	//                             from capture request settings */
	bool aeLock;
	bool awbLock;
	bool blackLevelLock;
	/*
	 * Manual color correction.
	 * This will be used to overwrite the results of PA
	 */
	ia_aiq_color_channels manualColorGains;
	float manualColorTransform[9];

private:
	/*!< ia_aiq_ae_input_params pointer contents */
	ia_aiq_exposure_sensor_descriptor sensorDescriptor;
	ia_rectangle exposureWindow;
	ia_coordinate exposureCoordinate;
	ia_aiq_ae_features aeFeatures;
	ia_aiq_ae_manual_limits aeManualLimits;
	long manual_exposure_time_us[NUM_EXPOSURES];
	float manual_analog_gain[NUM_EXPOSURES];
	short manual_iso[NUM_EXPOSURES];

	/*!< ia_aiq_af_input_params pointer contents */
	ia_aiq_manual_focus_parameters manualFocusParams;
	ia_rectangle focusRect;

	/*!< ia_aiq_awb_input_params pointer contents */
	ia_aiq_awb_manual_cct_range manualCctRange;
	ia_coordinate manualWhiteCoordinate;

	/*!< ia_aiq_pa_input_params pointer contents */
	ia_aiq_awb_results awbResults;
	ia_aiq_color_channels colorGains;
	ia_aiq_exposure_parameters exposureParams;

	/*!< ia_aiq_sa_input_params pointer contents*/
	ia_aiq_frame_params sensorFrameParams;
};

} /* namespace libcamera::ipa::ipu3::aiq */

#endif /* IPA_IPU3_AIQ_INPUT_PARAMETERS_H */
