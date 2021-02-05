/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * aiq.h - Intel IA Imaging library C++ wrapper
 *
 * To simplify naming, and prevent confusion the wrapper is named simply aiq
 * rather than ia_aiq.
 */

#include <string>

#include <ia_imaging/ia_aiq.h>
#include <ia_imaging/ia_cmc_parser.h>
#include <linux/intel-ipu3.h>

#include "aiq_input_parameters.h"
#include "aiq_results.h"
#include "binary_data.h"

#ifndef IPA_IPU3_AIQ_H
#define IPA_IPU3_AIQ_H

namespace libcamera {

class IPAIPU3Stats;

namespace ipa::ipu3::aiq {

class AIQ
{
public:
	AIQ();
	~AIQ();

	int init(BinaryData &aiqb, BinaryData &nvm, BinaryData &aiqd);
	int configure();
	int setStatistics(unsigned int frame, aiq::AiqResults &results,
			  const ipu3_uapi_stats_3a *stats);
	int run(unsigned int frame, aiq::AiqInputParameters &params,
		aiq::AiqResults &results);

private:
	std::string decodeError(ia_err err);

	int aeRun(ia_aiq_ae_input_params &aeParams,
		  aiq::AiqResults &results);
	int afRun(ia_aiq_af_input_params &afParams,
		  aiq::AiqResults &results);
	int afBracketRun(ia_aiq_af_bracket_input_params &afBracketParams,
			 aiq::AiqResults &results);
	int awbRun(ia_aiq_awb_input_params &awbParams,
		   aiq::AiqResults &results);
	int dsdRun(ia_aiq_dsd_input_params &dsdParams,
		   aiq::AiqResults &results);
	int gbceRun(ia_aiq_gbce_input_params &gbceParams,
		    aiq::AiqResults &results);
	int parameterAdapterRun(ia_aiq_pa_input_params &paParams,
				aiq::AiqResults &results);
	int shadingAdapterRun(ia_aiq_sa_input_params &saParams,
			      aiq::AiqResults &results);

	ia_aiq *aiq_;
	ia_cmc_t *iaCmc_;
	std::string version_;

	IPAIPU3Stats *aiqStats_;
};

} /* namespace ipa::ipu3::aiq */

} /* namespace libcamera */

#endif /* IPA_IPU3_AIQ_H */
