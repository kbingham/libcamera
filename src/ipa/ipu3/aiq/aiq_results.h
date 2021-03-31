/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * aiq_results.h - Intel IA Imaging library C++ wrapper
 *
 * AIQ results container, capable of depth copies and assignments
 * of the aiq result structures.
 */

#include <vector>

#include <ia_imaging/ia_aiq.h>

#ifndef IPA_IPU3_AIQ_RESULTS_H
#define IPA_IPU3_AIQ_RESULTS_H

namespace libcamera::ipa::ipu3::aiq {

static const unsigned int NUM_FLASH_LEDS = 1; /*!> Number of leds AEC algorithm
						     provides output for */

/**
 * The result structures for 3A algorithm are full of pointers to other structs,
 * some of those depends on the RGBS grid size or LSC grid size
 * We should query those at init time and initialize the struct with the correct
 * amount of memory. This is a TODO as an optimization
 * for now we just allocate statically big values.
 */
static const unsigned int MAX_AE_GRID_SIZE = 2048; /*!> Top limit for  the RGBS grid size
							This is an upper limit to avoid dynamic allocation*/
static const unsigned int DEFAULT_LSC_SIZE = 2048;
static const unsigned int MAX_GAMMA_LUT_SIZE = 1024;
static const unsigned int MAX_NUM_TONE_MAP_LUTS = 1024;

class AiqResults
{
public:
	AiqResults();

	const ia_aiq_ae_results *ae() { return &ae_; }
	ia_aiq_af_results *af() { return &af_; }
	const ia_aiq_af_bracket_results *afBracket() { return &afBracket_; }
	ia_aiq_awb_results *awb() { return &awb_; }
	const ia_aiq_gbce_results *gbce() { return &gbce_; }
	const ia_aiq_pa_results *pa() { return &pa_; }
	const ia_aiq_sa_results *sa() { return &sa_; }

	void setAe(ia_aiq_ae_results *ae);
	void setAf(ia_aiq_af_results *af);
	void setAfBracket(ia_aiq_af_bracket_results *afBracket);
	void setAwb(ia_aiq_awb_results *awb);
	void setGbce(ia_aiq_gbce_results *gbce);
	void setDetectedSceneMode(ia_aiq_scene_mode dsm);
	void setPa(ia_aiq_pa_results *pa);
	void setSa(ia_aiq_sa_results *sa);

private:
	ia_aiq_ae_results ae_;
	ia_aiq_af_results af_;
	ia_aiq_af_bracket_results afBracket_;
	ia_aiq_awb_results awb_;
	ia_aiq_gbce_results gbce_;
	ia_aiq_pa_results pa_;
	ia_aiq_sa_results sa_;

	ia_aiq_scene_mode detectedSceneMode_;

	/*!< ia_aiq_ae_results pointer contents */
	ia_aiq_ae_exposure_result aeExposureResult_;
	ia_aiq_hist_weight_grid aeWeightGrid_;
	unsigned char aeWeights_[MAX_AE_GRID_SIZE];
	ia_aiq_flash_parameters aeFlashes_[NUM_FLASH_LEDS];

	/*!< ia_aiq_ae_exposure_result pointer contents */
	ia_aiq_exposure_parameters aeExposureParameters_;
	ia_aiq_exposure_sensor_parameters aeSensorParaemeters_;

	/*!< ia_aiq_gbce results */
	/* The actual size of this table can be calculated by running cold
	 * GBCE, it will provide those tables. TODO!!
	 */
	float RGammaLut_[MAX_GAMMA_LUT_SIZE];
	float GGammaLut_[MAX_GAMMA_LUT_SIZE];
	float BGammaLut_[MAX_GAMMA_LUT_SIZE];
	float toneMapLut_[MAX_NUM_TONE_MAP_LUTS];

	/*!< ia_aiq_af_bracket_results pointer contents */
	unsigned short distanceBracketing_;
	int lensPosBracketing_;

	/*!< ia_aiq_pa_results */
	ia_aiq_advanced_ccm_t prefAcm_;
	ia_aiq_ir_weight_t irWeight_;

	/*!< ia_aiq_advanced_ccm_t pointer contents */
	unsigned int hueOfSectors_;
	float advancedColorConversionMatrices_[3][3];

	/*!< ia_aiq_ir_weight_t pointer contents */
	unsigned short irWeightGridR_;
	unsigned short irWeightGridG_;
	unsigned short irWeightGridB_;

	/*!< ia_aiq_sa_results pointer contents */
	std::vector<float> channelGr_;
	std::vector<float> channelR_;
	std::vector<float> channelB_;
	std::vector<float> channelGb_;
};

} /* namespace libcamera::ipa::ipu3::aiq */

#endif /* IPA_IPU3_AIQ_RESULTS_H */

