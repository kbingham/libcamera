/* SPDX-License-Identifier: Apache-2.0 */
/*
 * IPU3ISPPipe: Derived class from ISPPipe storing the AIC config
 */

#ifndef IPU3ISPPIPE_H
#define IPU3ISPPIPE_H

/* Included first to bring in our type wrapper */
#include "kbl_aic.h"

#include "IPU3AICCommon.h"
#include "Pipe.h"

namespace libcamera::ipa::ipu3::aic {

#define NUM_ISP_PIPES 1

typedef enum AicMode {
	AIC_MODE_STILL = 0,
	AIC_MODE_VIDEO,
	AIC_MODE_MAX,
} AicMode;

class IPU3ISPPipe : public ISPPipe
{
public:
	IPU3ISPPipe();

public:
	// This function configures the HW/FW pipe via CSS interface
	virtual void SetPipeConfig(const aic_output_t pipe_config);

	virtual pipe_ver GetPipeVer() { return Czero; }

	virtual const ia_aiq_rgbs_grid *GetAWBStats();
	virtual const ia_aiq_af_grid *GetAFStats();
	virtual const ia_aiq_histogram *GetAEStats();
	virtual aic_config *GetAicConfig();

	virtual void dump();

private:
	aic_output_t AicOutput;

	aic_config AicConfig; /* Config to driver */
};

} /* namespace libcamera::ipa::ipu3::aic */

#endif /* IPU3ISPPIPE_H */
