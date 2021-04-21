/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * aiq_results.cpp - Intel IA Imaging library C++ wrapper
 *
 * AIQ results container, capable of depth copies and assignments
 * of the aiq result structures.
 */

#include "aiq/aiq_results.h"

#include <algorithm>

#include "libcamera/internal/log.h"

/* Macros used by imported code */
#define STDCOPY(dst, src, size) std::copy((src), ((src) + (size)), (dst))

namespace libcamera {

LOG_DEFINE_CATEGORY(AIQResults)

namespace ipa {
namespace ipu3 {
namespace aiq {

AiqResults::AiqResults()
{
	/* Initialise AE */
	ae_.exposures = &aeExposureResult_;
	ae_.exposures->exposure = &aeExposureParameters_;
	ae_.exposures->sensor_exposure = &aeSensorParaemeters_;
	ae_.weight_grid = &aeWeightGrid_;
	ae_.weight_grid->weights = aeWeights_;
	ae_.flashes = aeFlashes_;

	/* GBCE */
	gbce_.r_gamma_lut = RGammaLut_;
	gbce_.g_gamma_lut = GGammaLut_;
	gbce_.b_gamma_lut = BGammaLut_;
	gbce_.gamma_lut_size = MAX_GAMMA_LUT_SIZE;
	gbce_.tone_map_lut = toneMapLut_;
	gbce_.tone_map_lut_size = MAX_NUM_TONE_MAP_LUTS;

	/* Initialise afBracket */
	afBracket_.distances_bracketing = &distanceBracketing_;
	afBracket_.lens_positions_bracketing = &lensPosBracketing_;

	/* Initialise PA */
	pa_.preferred_acm = &prefAcm_;
	pa_.preferred_acm->hue_of_sectors = &hueOfSectors_;
	pa_.preferred_acm->advanced_color_conversion_matrices =
		&advancedColorConversionMatrices_;
	pa_.ir_weight = &irWeight_;
	pa_.ir_weight->ir_weight_grid_R = &irWeightGridR_;
	pa_.ir_weight->ir_weight_grid_G = &irWeightGridG_;
	pa_.ir_weight->ir_weight_grid_B = &irWeightGridB_;

	/* Initialise SA */
	channelGr_.reserve(DEFAULT_LSC_SIZE);
	channelGb_.reserve(DEFAULT_LSC_SIZE);
	channelR_.reserve(DEFAULT_LSC_SIZE);
	channelB_.reserve(DEFAULT_LSC_SIZE);
	sa_.channel_gr = channelGr_.data();
	sa_.channel_gb = channelGb_.data();
	sa_.channel_r = channelR_.data();
	sa_.channel_b = channelB_.data();
}

void AiqResults::setAe(ia_aiq_ae_results *ae)
{
	/* Todo: Potentially Requires copying
	 *   ia_aiq_aperture_control *aperture_control;
	 */

	ASSERT(ae);

	if (!ae) {
		LOG(AIQResults, Error) << "Invalid AE argument";
		return;
	}

	ae_.lux_level_estimate = ae->lux_level_estimate;
	ae_.flicker_reduction_mode = ae->flicker_reduction_mode;
	ae_.multiframe = ae->multiframe;
	ae_.num_flashes = ae->num_flashes;
	ae_.num_exposures = ae->num_exposures;

	ae_.exposures->converged = ae->exposures->converged;
	ae_.exposures->distance_from_convergence = ae->exposures->distance_from_convergence;
	ae_.exposures->exposure_index = ae->exposures->exposure_index;

	if (ae_.exposures->exposure && ae->exposures->exposure) {
		*ae_.exposures->exposure = *ae->exposures->exposure;
	} else {
		LOG(AIQResults, Error) << "Not copying AE Exposure";
	}

	if (ae_.exposures->sensor_exposure && ae->exposures->sensor_exposure) {
		*ae_.exposures->sensor_exposure = *ae->exposures->sensor_exposure;
	} else {
		LOG(AIQResults, Error) << "Not copying AE Sensor Exposure";
	}

	// Copy weight grid
	if (ae_.weight_grid && ae->weight_grid &&
	    ae_.weight_grid->weights && ae->weight_grid->weights) {
		ae_.weight_grid->width = ae->weight_grid->width;
		ae_.weight_grid->height = ae->weight_grid->height;

		unsigned int gridElements = ae->weight_grid->width *
					    ae->weight_grid->height;
		gridElements = std::clamp<unsigned int>(gridElements, 1, MAX_AE_GRID_SIZE);

		STDCOPY(ae_.weight_grid->weights,
			ae->weight_grid->weights,
			gridElements * sizeof(char));
	} else {
		LOG(AIQResults, Error) << "Not copying AE Weight Grids";
	}

	// Copy the flash info structure
	if (ae_.flashes && ae->flashes) {
		STDCOPY((int8_t *)ae_.flashes, (int8_t *)ae->flashes,
			NUM_FLASH_LEDS * sizeof(ia_aiq_flash_parameters));
	} else {
		LOG(AIQResults, Error) << "Not copying AE Flashes";
	}
}

void AiqResults::setAf(ia_aiq_af_results *af)
{
	ASSERT(af);

	af_.status = af->status;
	af_.current_focus_distance = af->current_focus_distance;
	af_.next_lens_position = af->next_lens_position;
	af_.lens_driver_action = af->lens_driver_action;
	af_.use_af_assist = af->use_af_assist;
	af_.final_lens_position_reached = af->final_lens_position_reached;
}

void AiqResults::setAfBracket(ia_aiq_af_bracket_results *afBracket)
{
	ASSERT(afBracket);

	afBracket_.distances_bracketing = afBracket->distances_bracketing;
	afBracket_.lens_positions_bracketing = afBracket->lens_positions_bracketing;
}

void AiqResults::setAwb(ia_aiq_awb_results *awb)
{
	ASSERT(awb);

	awb_.accurate_r_per_g = awb->accurate_r_per_g;
	awb_.accurate_b_per_g = awb->accurate_b_per_g;
	awb_.final_r_per_g = awb->final_r_per_g;
	awb_.final_b_per_g = awb->final_b_per_g;
	awb_.cct_estimate = awb->cct_estimate;
	awb_.distance_from_convergence = awb->distance_from_convergence;
}

void AiqResults::setGbce(ia_aiq_gbce_results *gbce)
{
	ASSERT(gbce);

	if (gbce->gamma_lut_size > 0) {
		ASSERT(gbce->gamma_lut_size <= MAX_GAMMA_LUT_SIZE);

		gbce_.gamma_lut_size = gbce->gamma_lut_size;

		STDCOPY((int8_t *)gbce_.r_gamma_lut, (int8_t *)gbce->r_gamma_lut,
			gbce->gamma_lut_size * sizeof(float));
		STDCOPY((int8_t *)gbce_.b_gamma_lut, (int8_t *)gbce->b_gamma_lut,
			gbce->gamma_lut_size * sizeof(float));
		STDCOPY((int8_t *)gbce_.g_gamma_lut, (int8_t *)gbce->g_gamma_lut,
			gbce->gamma_lut_size * sizeof(float));
	} else {
		LOG(AIQResults, Error) << "Not copying Gamma LUT channels";
	}

	if (gbce->tone_map_lut_size > 0) {
		gbce_.tone_map_lut_size = gbce->tone_map_lut_size;
		STDCOPY((int8_t *)gbce_.tone_map_lut, (int8_t *)gbce->tone_map_lut,
			gbce->tone_map_lut_size * sizeof(float));
	} else {
		LOG(AIQResults, Error) << "Not copying Tone Mapping Gain LUT";
	}
}

void AiqResults::setDetectedSceneMode(ia_aiq_scene_mode dsm)
{
	detectedSceneMode_ = dsm;
}

void AiqResults::setPa(ia_aiq_pa_results *pa)
{
	ASSERT(pa);

	STDCOPY(&pa_.color_conversion_matrix[0][0], &pa->color_conversion_matrix[0][0],
		MAX_COLOR_CONVERSION_MATRIX * MAX_COLOR_CONVERSION_MATRIX *
			sizeof(pa->color_conversion_matrix[0][0]));

	if (pa_.preferred_acm && pa->preferred_acm) {
		pa_.preferred_acm->sector_count = pa->preferred_acm->sector_count;

		STDCOPY(pa_.preferred_acm->hue_of_sectors,
			pa->preferred_acm->hue_of_sectors,
			sizeof(*pa->preferred_acm->hue_of_sectors) * pa->preferred_acm->sector_count);

		STDCOPY(pa_.preferred_acm->advanced_color_conversion_matrices[0][0],
			pa->preferred_acm->advanced_color_conversion_matrices[0][0],
			sizeof(*pa->preferred_acm->advanced_color_conversion_matrices) * pa->preferred_acm->sector_count);
	} else {
		LOG(AIQResults, Error) << "Not copying PA hue of sectors";
	}

	if (pa_.ir_weight && pa->ir_weight) {
		pa_.ir_weight->height = pa->ir_weight->height;
		pa_.ir_weight->width = pa->ir_weight->width;

		STDCOPY(pa_.ir_weight->ir_weight_grid_R,
			pa->ir_weight->ir_weight_grid_R,
			sizeof(*pa->ir_weight->ir_weight_grid_R) * pa->ir_weight->height * pa->ir_weight->width);

		STDCOPY(pa_.ir_weight->ir_weight_grid_G,
			pa->ir_weight->ir_weight_grid_G,
			sizeof(*pa->ir_weight->ir_weight_grid_G) * pa->ir_weight->height * pa->ir_weight->width);

		STDCOPY(pa_.ir_weight->ir_weight_grid_B,
			pa->ir_weight->ir_weight_grid_B,
			sizeof(*pa->ir_weight->ir_weight_grid_B) * pa->ir_weight->height * pa->ir_weight->width);
	} else {
		LOG(AIQResults, Error) << "Not copying IR weight";
	}

	pa_.black_level = pa->black_level;
	pa_.color_gains = pa->color_gains;
	pa_.linearization = pa->linearization;
	pa_.saturation_factor = pa->saturation_factor;
	pa_.brightness_level = pa->brightness_level;
}

void AiqResults::setSa(ia_aiq_sa_results *sa)
{
	ASSERT(sa && sa->channel_r && sa->channel_gr &&
	       sa->channel_gb && sa->channel_b);

	sa_.width = sa->width;
	sa_.height = sa->height;
	sa_.lsc_update = sa->lsc_update;

	/* Check against one of the vectors but resize applicable to all. */
	if (channelGr_.size() < (sa_.width * sa_.height)) {
		int lscNewSize = sa_.width * sa_.height;
		channelGr_.resize(lscNewSize);
		channelGb_.resize(lscNewSize);
		channelR_.resize(lscNewSize);
		channelB_.resize(lscNewSize);

		/* Update the SA data pointers to new memory locations. */
		sa_.channel_gr = channelGr_.data();
		sa_.channel_gb = channelGb_.data();
		sa_.channel_r = channelR_.data();
		sa_.channel_b = channelB_.data();
	}

	if (sa->lsc_update) {
		uint32_t memCopySize = sa->width * sa->height * sizeof(float);

		STDCOPY((int8_t *)sa_.channel_gr, (int8_t *)sa->channel_gb,
			memCopySize);
		STDCOPY((int8_t *)sa_.channel_gb, (int8_t *)sa->channel_gr,
			memCopySize);
		STDCOPY((int8_t *)sa_.channel_r, (int8_t *)sa->channel_r,
			memCopySize);
		STDCOPY((int8_t *)sa_.channel_b, (int8_t *)sa->channel_b,
			memCopySize);
	} else {
		LOG(AIQResults, Error) << "Not copying LSC tables.";
	}

	STDCOPY(&sa_.light_source[0],
		&sa->light_source[0],
		CMC_NUM_LIGHTSOURCES * sizeof(sa->light_source[0]));
	sa_.scene_difficulty = sa->scene_difficulty;
	sa_.num_patches = sa->num_patches;
	sa_.covered_area = sa->covered_area;
	sa_.frame_params = sa->frame_params;
}

} /* namespace aiq */
} /* namespace ipu3 */
} /* namespace ipa */

} /* namespace libcamera */
