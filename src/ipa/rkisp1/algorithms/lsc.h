/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021-2022, Ideas On Board
 *
 * RkISP1 Lens Shading Correction control
 */

#pragma once

#include <map>
#include <memory>

#include "libipa/interpolator.h"
#include "libipa/lsc_base.h"

#include "algorithm.h"

namespace libcamera {

namespace ipa::rkisp1::algorithms {

class LensShadingCorrection : public Algorithm
{
public:
	LensShadingCorrection();
	~LensShadingCorrection() = default;

	int init(IPAContext &context, const ValueNode &tuningData) override;
	int configure(IPAContext &context, const IPACameraSensorInfo &configInfo) override;
	void queueRequest(IPAContext &context, const uint32_t frame,
			  IPAFrameContext &frameContext,
			  const ControlList &controls) override;
	void prepare(IPAContext &context, const uint32_t frame,
		     IPAFrameContext &frameContext,
		     RkISP1Params *params) override;
	void process(IPAContext &context, const uint32_t frame,
		     IPAFrameContext &frameContext,
		     const rkisp1_stat_buffer *stats,
		     ControlList &metadata) override;
private:
	void setParameters(rkisp1_cif_isp_lsc_config &config);
	void copyTable(rkisp1_cif_isp_lsc_config &config, const lsc::Components &set0);

	std::vector<double> xSize_;
	std::vector<double> ySize_;
	uint16_t xGrad_[RKISP1_CIF_ISP_LSC_SECTORS_TBL_SIZE];
	uint16_t yGrad_[RKISP1_CIF_ISP_LSC_SECTORS_TBL_SIZE];
	uint16_t xSizes_[RKISP1_CIF_ISP_LSC_SECTORS_TBL_SIZE];
	uint16_t ySizes_[RKISP1_CIF_ISP_LSC_SECTORS_TBL_SIZE];

	unsigned int lastAppliedCt_;
	unsigned int lastAppliedQuantizedCt_;

	ipa::Interpolator<lsc::Components> sets_;

	std::unique_ptr<LscImplementation> algo_;
};

} /* namespace ipa::rkisp1::algorithms */
} /* namespace libcamera */
