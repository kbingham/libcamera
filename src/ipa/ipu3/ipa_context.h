/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * ipu3_ipa_context.h - IPU3 IPA Context
 *
 * Context information shared between the algorithms
 */
#ifndef __LIBCAMERA_IPU3_IPA_CONTEXT_H__
#define __LIBCAMERA_IPU3_IPA_CONTEXT_H__

#include <linux/intel-ipu3.h>

#include <libcamera/geometry.h>

namespace libcamera {

namespace ipa::ipu3 {

/* Fixed configuration of the IPA */
struct IPAConfiguration {
	struct Grid {
		/* Bayer Down Scaler grid plane config used by the kernel */
		ipu3_uapi_grid_config bdsGrid;
		/* BDS output size configured by the pipeline handler */
		Size bdsOutputSize;
	} grid;
};

/*
 * Context of a frame for each algorithms
 * This may be stored in a way that is associated with a given request
 * lifetime, though for now a single instance is used.
 */
struct IPAFrameContext {
};

/* Global context of the IPA */
struct IPAContext {
	IPAConfiguration configuration;
	IPAFrameContext frameContext;
};

} /* namespace ipa::ipu3 */

} /* namespace libcamera*/

#endif /* __LIBCAMERA_IPU3_IPA_CONTEXT_H__ */
