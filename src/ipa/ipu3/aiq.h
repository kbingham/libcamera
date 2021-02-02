/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * aiq.h - Intel IA Imaging library C++ wrapper
 *
 * To simplify naming, and prevent confusion the wrapper is named simply aiq
 * rather than ia_aiq.
 *
 * Bikeshedding opportunity: We could also name this
 *	ia_aiq++
 *	ia_aiqpp
 *	ia_aiq_wrapper
 *	...
 */

/* Intel IA AIQ support */
#include <ia_imaging/ia_aiq.h>

namespace libcamera {

class AIQ
{
public:
	AIQ();
	~AIQ();

	int init();
	int configure();

private:
	ia_aiq *aiq_;
};

} /* namespace libcamera */
