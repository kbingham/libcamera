/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Ideas On Board
 *
 * ipu3_awb.cpp - AWB control algorithm
 */
#include "ipu3_awb.h"

#include <cmath>
#include <numeric>
#include <unordered_map>

#include "libcamera/internal/log.h"

namespace libcamera {

namespace ipa {

LOG_DEFINE_CATEGORY(IPU3Awb)

static const struct ipu3_uapi_bnr_static_config imguCssBnrDefaults = {
	.wb_gains = { 16, 16, 16, 16 },
	.wb_gains_thr = { 255, 255, 255, 255 },
	.thr_coeffs = { 1700, 0, 31, 31, 0, 16 },
	.thr_ctrl_shd = { 26, 26, 26, 26 },
	.opt_center{ -648, 0, -366, 0 },
	.lut = {
		{ 17, 23, 28, 32, 36, 39, 42, 45,
		  48, 51, 53, 55, 58, 60, 62, 64,
		  66, 68, 70, 72, 73, 75, 77, 78,
		  80, 82, 83, 85, 86, 88, 89, 90 } },
	.bp_ctrl = { 20, 0, 1, 40, 0, 6, 0, 6, 0 },
	.dn_detect_ctrl{ 9, 3, 4, 0, 8, 0, 1, 1, 1, 1, 0 },
	.column_size = 1296,
	.opt_center_sqr = { 419904, 133956 },
};

/* settings for Auto White Balance */
static const struct ipu3_uapi_awb_config_s imguCssAwbDefaults = {
	.rgbs_thr_gr = 8191,
	.rgbs_thr_r = 8191,
	.rgbs_thr_gb = 8191,
	.rgbs_thr_b = 8191 | IPU3_UAPI_AWB_RGBS_THR_B_EN | IPU3_UAPI_AWB_RGBS_THR_B_INCL_SAT,
	.grid = {
		.width = 129,
		.height = 36,
		.block_width_log2 = 3,
		.block_height_log2 = 4,
		.height_per_slice = 1, /* Overridden by kernel. */
		.x_start = 0,
		.y_start = 0,
		.x_end = 0,
		.y_end = 0,
	},
};

static const struct ipu3_uapi_ccm_mat_config imguCssCcm6000k = {
	6000, 0, 0, 0,
	0, 4000, 0, 0,
	0, 0, 10000, 0
};

static const struct ipu3_uapi_ccm_mat_config imguCssCcm3800k = {
	9802/*6326*/, 0, 0, 0,
	0, 10500/*8192*/, 0, 0,
	0, 0, 15137/*9671*/, 0
};

IPU3Awb::IPU3Awb()
	: Algorithm()
{
}

IPU3Awb::~IPU3Awb()
{
}

void IPU3Awb::initialise(ipu3_uapi_params &params, const Size &bds)
{
	LOG(IPU3Awb, Debug) << "BDS passed at configure: ("
			    << bds.width << ","
			    << bds.height << ")";

	params.use.acc_awb = 1;
	/*\todo fill the grid calculated based on BDS configuration */
	params.acc_param.awb.config = imguCssAwbDefaults;

	/*\todo pass this grid somehow to other algorithms (calculated in IPAIPU3 ?)
	 * It needs to be calculated properly */
	bdsGrid_.width = 129;
	bdsGrid_.block_width_log2 = 3;
	bdsGrid_.height = 36;
	bdsGrid_.block_height_log2 = 4;

	params.acc_param.awb.config.grid.width = bdsGrid_.width;
	params.acc_param.awb.config.grid.block_width_log2 = bdsGrid_.block_width_log2;
	params.acc_param.awb.config.grid.height = bdsGrid_.height;
	params.acc_param.awb.config.grid.block_height_log2 = bdsGrid_.block_height_log2;

	params.use.obgrid = 0;
	params.obgrid_param.gr = 20;
	params.obgrid_param.r = 28;
	params.obgrid_param.b = 28;
	params.obgrid_param.gb = 20;

	params.use.acc_bnr = 1;
	params.acc_param.bnr = imguCssBnrDefaults;

	params.use.acc_ccm = 1;
	params.acc_param.ccm = imguCssCcm6000k;

	params.use.acc_gamma = 1;
	params.acc_param.gamma.gc_ctrl.enable = 1;

	params.use.acc_green_disparity = 0;
	params.acc_param.green_disparity.gd_black = 2440;
	params.acc_param.green_disparity.gd_red = 4;
	params.acc_param.green_disparity.gd_blue = 4;
	params.acc_param.green_disparity.gd_green = 4;
	params.acc_param.green_disparity.gd_shading = 24;
	params.acc_param.green_disparity.gd_support = 2;
	params.acc_param.green_disparity.gd_clip = 1;
	params.acc_param.green_disparity.gd_central_weight = 5;

	params.use.acc_cds = 1;
	params.acc_param.cds.csc_en = 1;
	params.acc_param.cds.uv_bin_output = 0;
	params.acc_param.cds.ds_c00 = 0;
	params.acc_param.cds.ds_c01 = 1;
	params.acc_param.cds.ds_c02 = 1;
	params.acc_param.cds.ds_c03 = 0;
	params.acc_param.cds.ds_c10 = 0;
	params.acc_param.cds.ds_c11 = 1;
	params.acc_param.cds.ds_c12 = 1;
	params.acc_param.cds.ds_c13 = 0;
	params.acc_param.cds.ds_nf = 2;

	wbGains_[0] = 16;
	wbGains_[1] = 4096;
	wbGains_[2] = 4096;
	wbGains_[3] = 16;

	frame_count_ = 0;
}

uint32_t IPU3Awb::estimateCCT(uint8_t red, uint8_t green, uint8_t blue)
{
	double X = (-0.14282) * (red) + (1.54924) * (green) + (-0.95641) * (blue);
	double Y = (-0.32466) * (red) + (1.57837) * (green) + (-0.73191) * (blue);
	double Z = (-0.68202) * (red) + (0.77073) * (green) + (0.56332) * (blue);

	double x = X / (X + Y + Z);
	double y = Y / (X + Y + Z);

	double n = (x - 0.3320) / (0.1858 - y);
	return static_cast<uint32_t>(449 * n * n * n + 3525 * n * n + 6823.3 * n + 5520.33);
}

double meanValue(std::vector<uint32_t> colorValues)
{
	uint32_t count = 0;
	uint32_t hist[256] = { 0 };
	for (uint32_t const &val : colorValues) {
		hist[val]++;
		count++;
	}

	double mean = 0.0;
	for (uint32_t i = 0; i < 256; i++) {
		mean += hist[i] * i;
	}
	return mean /= count;
}

void IPU3Awb::calculateWBGains(Rectangle roi, const ipu3_uapi_stats_3a *stats)
{
	std::vector<uint32_t> redValues, greenValues, blueValues;
	Point topleft = roi.topLeft();
	uint32_t startY = (topleft.y / 16) * 129 * 8;
	uint32_t startX = (topleft.x / 8) * 8;
	uint32_t endX = (startX + (roi.size().width / 8)) * 8;

	for (uint32_t j = (topleft.y / 16); j < (topleft.y / 16) + (roi.size().height / 16); j++) {
		for (uint32_t i = startX + startY; i < endX + startY; i += 8) {
			greenValues.push_back(stats->awb_raw_buffer.meta_data[i + j * 129]);
			redValues.push_back(stats->awb_raw_buffer.meta_data[i + 1 + j * 129]);
			blueValues.push_back(stats->awb_raw_buffer.meta_data[i + 2 + j * 129]);
			greenValues.push_back(stats->awb_raw_buffer.meta_data[i + 3 + j * 129]);
		}
	}

	double rMean = meanValue(redValues);
	double bMean = meanValue(blueValues);
	double gMean = meanValue(greenValues);

	double rGain = gMean / rMean;
	double bGain = gMean / bMean;

	wbGains_[0] = 16;
	wbGains_[1] = 4096 * rGain;
	wbGains_[2] = 4096 * bGain;
	wbGains_[3] = 16;

	frame_count_++;

	cct_ = estimateCCT(rMean, gMean, bMean);
}

void IPU3Awb::updateWbParameters(ipu3_uapi_params &params, double agcGamma)
{
	if ((wbGains_[0] == 0) || (wbGains_[1] == 0) || (wbGains_[2] == 0) || (wbGains_[3] == 0)) {
		LOG(IPU3Awb, Error) << "Gains can't be 0, check the stats";
	} else {
		params.acc_param.bnr.wb_gains.gr = wbGains_[0];
		params.acc_param.bnr.wb_gains.r = wbGains_[1];
		params.acc_param.bnr.wb_gains.b = wbGains_[2];
		params.acc_param.bnr.wb_gains.gb = wbGains_[3];

		LOG(IPU3Awb, Debug) << "Color temperature estimated: " << cct_
				    << " and gamma calculated: " << agcGamma;
		params.acc_param.ccm = imguCssCcm3800k;

		for (uint32_t i = 0; i < 256; i++) {
			double j = i / 255.0;
			double gamma = std::pow(j, 1.0 / agcGamma);
			params.acc_param.gamma.gc_lut.lut[i] = gamma * 8191;
		}
	}
}

} /* namespace ipa */

} /* namespace libcamera */
