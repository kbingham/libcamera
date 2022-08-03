/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Ideas on Board Oy.
 *
 * shd.h - IPU3 Lens Shading Correction
 */

#pragma once

#include "algorithm.h"

namespace libcamera {

namespace ipa::ipu3::algorithms {

class LensShadingCorrection : public Algorithm
{
public:
	LensShadingCorrection();

	int init(IPAContext &context, const YamlObject &tuningData) override;
	void prepare(IPAContext &context, ipu3_uapi_params *params) override;

private:
	bool initialized_ = false;

	Size gridSize_;
	Size gridBlockSize_;
	unsigned int gain_;
	unsigned int x_, y_;

	std::vector<uint16_t> rData_;
	std::vector<uint16_t> grData_;
	std::vector<uint16_t> gbData_;
	std::vector<uint16_t> bData_;

	std::vector<double> xSize_;
	std::vector<double> ySize_;
};

} /* namespace ipa::ipu3::algorithms */

} /* namespace libcamera */
