/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Incorporate equivalent structures for the AIC algorithms without requiring
 * all of the header based implementation provided by the IA AIQ library
 * headers.
 */

#include <string>

#include <libcamera/geometry.h>

#include "kbl_aic.h"

#include "aiq/aiq_results.h"
#include "ipu3_isp_pipe.h"

#ifndef IPA_IPU3_AIC_H
#define IPA_IPU3_AIC_H

namespace libcamera::ipa::ipu3::aic {

class AIC
{
public:
	~AIC();

	int init();
	void reset();
	int run();
	aic_config_t *GetAicConfig();
	void updateRuntimeParams(ipa::ipu3::aiq::AiqResults &results);
	std::string version();
	int configure(const Size bds, const Size ifSize, const Size gdcSize,
		      const Size cropRegion_);

private:
	/** \todo: Only a single AIC_MODE is supported currently. */
	std::unique_ptr<KBL_AIC> skyCam_;

	ia_cmc_t *iaCmc_;

	//std::unique_ptr<IPU3ISPPipe> mIspPipes[AIC_MODE_MAX][NUM_ISP_PIPES];
	//std::unique_ptr<KBL_AIC> mSkyCam[AIC_MODE_MAX];

	/* IPU3AICRuntimeParams pointer contents */
	ia_aiq_output_frame_parameters_t mRuntimeParamsOutFrameParams_;
	aic_resolution_config_parameters_t mRuntimeParamsResCfgParams_;
	aic_input_frame_parameters_t mRuntimeParamsInFrameParams_;
	ia_rectangle mRuntimeParamsRec_;
	IPU3AICRuntimeParams mRuntimeParams_;

	IPU3ISPPipe *pipe_;
};

} /* namespace libcamera::ipa::ipu3::aic */

#endif /* IPA_IPU3_AIC_H */

