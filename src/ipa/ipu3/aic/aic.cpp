/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * aic.cpp - Intel IA Imaging library C++ wrapper
 *
 * Automatic IPU Configuration
 */

#include <ia_imaging/ia_cmc_parser.h>

#include "aic.h"

#include "libcamera/internal/log.h"

#include "../aiq/binary_data.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define ALIGN128(x) (((x) + 127) & ~127)

namespace libcamera {

LOG_DEFINE_CATEGORY(AIC)

namespace ipa::ipu3::aic {

/*
 * Only a Single Pipeline instance of the AIC is currently supported.
 * The CrOS implementation defines a set of AIC to run for both STILL and VIDEO
 * allowing improved perfomance on preview streams while taking an image
 * capture.
 */

AIC::~AIC()
{
	if (iaCmc_)
		ia_cmc_parser_deinit(iaCmc_);

	delete pipe_;
}

int AIC::init()
{
	LOG(AIC, Debug) << "Initialising IA AIC Wrapper";

	pipe_ = new IPU3ISPPipe();

	CLEAR(mRuntimeParamsOutFrameParams_);
	CLEAR(mRuntimeParamsResCfgParams_);
	CLEAR(mRuntimeParamsInFrameParams_);
	CLEAR(mRuntimeParamsRec_);
	CLEAR(mRuntimeParams_);
	mRuntimeParams_.output_frame_params = &mRuntimeParamsOutFrameParams_;
	mRuntimeParams_.frame_resolution_parameters = &mRuntimeParamsResCfgParams_;
	mRuntimeParams_.input_frame_params = &mRuntimeParamsInFrameParams_;
	mRuntimeParams_.focus_rect = &mRuntimeParamsRec_;

	ipa::ipu3::aiq::BinaryData aiqb;
	int ret = aiqb.load("/etc/camera/ipu3/00imx258.aiqb");
	if (ret) {
		LOG(AIC, Error) << "Failed to load AIQB";
		return -ENODATA;
	}
	iaCmc_ = ia_cmc_parser_init(aiqb.data());

	/* \todo: Initialise the mRuntimeParams with ia_aiq_frame_params before
	 * constructing the KBL_AIC.
	 * In CrOS, GraphConfig::getSensorFrameParams provides all these
	 * details. Start looking from ParameterWorker::configure()
	 */
	ISPPipe *pipe = static_cast<ISPPipe *>(pipe_);
	skyCam_ = std::make_unique<KBL_AIC>(&pipe, 1, iaCmc_, aiqb.data(),
					    mRuntimeParams_, 0, 0);

	return 0;
}

void AIC::reset()
{
}

int AIC::run()
{
	LOG(AIC, Debug) << "IA AIC Run()";
	skyCam_->Run(&mRuntimeParams_, 1);
	/* \todo: Get the AicConfig here. The output config should be run
	 * down through the ParameterEncoder to get h/w params.
	 */
	return 0;
}

std::string AIC::version()
{
	return "";
}

aic_config_t *AIC::GetAicConfig()
{
	pipe_->dump();
	return pipe_->GetAicConfig();
}

void AIC::updateRuntimeParams(ipa::ipu3::aiq::AiqResults &results)
{
	mRuntimeParams_.pa_results = results.pa();
	mRuntimeParams_.sa_results = results.sa();

	const ia_aiq_ae_results *ae = results.ae();
	mRuntimeParams_.exposure_results = ae->exposures->exposure;
	mRuntimeParams_.weight_grid = ae->weight_grid;

	mRuntimeParams_.isp_vamem_type = 0;
	mRuntimeParams_.awb_results = results.awb();
	mRuntimeParams_.gbce_results = results.gbce();

	/* \todo: Set below parameters from capture settings
	params->time_stamp = 0; //microsecond unit
	params->manual_brightness = settings->ispSettings.manualSettings.manualBrightness;
	params->manual_contrast = settings->ispSettings.manualSettings.manualContrast;
	params->manual_hue = settings->ispSettings.manualSettings.manualHue;
	params->manual_saturation = settings->ispSettings.manualSettings.manualSaturation;
	params->manual_sharpness = settings->ispSettings.manualSettings.manualSharpness;
	*/
}

} /* namespace ipa::ipu3::aic */

} /* namespace libcamera */
