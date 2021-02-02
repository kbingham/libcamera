/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Incorporate equivalent structures for the AIC algorithms without requiring
 * all of the header based implementation provided by the IA AIQ library
 * headers.
 */

#include <string>

#include "kbl_aic.h"

#ifndef IPA_IPU3_AIC_H
#define IPA_IPU3_AIC_H

namespace libcamera::ipa::ipu3::aic {

class AIC
{
	int init();
	void reset();
	int run();
	std::string version();

private:
	/** \todo: Only a single AIC_MODE is supported currently. */
	std::unique_ptr<KBL_AIC> skyCam_;

	//std::unique_ptr<IPU3ISPPipe> mIspPipes[AIC_MODE_MAX][NUM_ISP_PIPES];
	//std::unique_ptr<KBL_AIC> mSkyCam[AIC_MODE_MAX];
};

} /* namespace libcamera::ipa::ipu3::aic */

#endif /* IPA_IPU3_AIC_H */
