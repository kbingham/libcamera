/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * aic.cpp - Intel IA Imaging library C++ wrapper
 *
 * Automatic IPU Configuration
 */

#include "aic.h"

#include "libcamera/internal/log.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(AIC)

namespace ipa::ipu3::aic {

/*
 * Only a Single Pipeline instance of the AIC is currently supported.
 * The CrOS implementation defines a set of AIC to run for both STILL and VIDEO
 * allowing improved perfomance on preview streams while taking an image
 * capture.
 */

int AIC::init()
{
	LOG(AIC, Debug) << "Initialising IA AIC Wrapper";

	return 0;
}

void AIC::reset()
{
}

int AIC::run()
{
	return 0;
}

std::string AIC::version()
{
	return "";
}

} /* namespace ipa::ipu3::aic */

} /* namespace libcamera */
