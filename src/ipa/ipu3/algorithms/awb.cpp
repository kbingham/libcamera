/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Ideas On Board
 *
 * awb.cpp - AWB control algorithm
 */
#include "awb.h"

#include <algorithm>
#include <cmath>

#include <libcamera/base/log.h>

namespace libcamera {

namespace ipa::ipu3::algorithms {

LOG_DEFINE_CATEGORY(IPU3Awb)

static constexpr uint32_t kMinZonesCounted = 16;
static constexpr uint32_t kMinGreenLevelInZone = 32;

/**
 * \struct Accumulator
 * \brief RGB statistics for a given zone
 *
 * - Cells are defined in Pixels
 * - Zones are defined in Cells
 *
 *                     81 cells
 *     /───────────── 1296 pixels ───────────\
 *                      16 zones
 *      16
 *    ┌────┬────┬────┬────┬────┬─  ──────┬────┐   \
 *    │Cell│    │    │    │    │    |    │    │   │
 * 16 │ px │    │    │    │    │    |    │    │   │
 *    ├────┼────┼────┼────┼────┼─  ──────┼────┤   │
 *    │    │    │    │    │    │    |    │    │
 *    │    │    │    │    │    │    |    │    │   7
 *    │ ── │ ── │ ── │ ── │ ── │ ──  ── ─┤ ── │ 1 2 4
 *    │    │    │    │    │    │    |    │    │ 2 0 5
 *
 *    │    │    │    │    │    │    |    │    │ z p c
 *    ├────┼────┼────┼────┼────┼─  ──────┼────┤ o i e
 *    │    │    │    │    │    │    |    │    │ n x l
 *    │                        │    |    │    │ e e l
 *    ├───                  ───┼─  ──────┼────┤ s l s
 *    │                        │    |    │    │   s
 *    │                        │    |    │    │
 *    ├───   Zone of Cells  ───┼─  ──────┼────┤   │
 *    │        (5 x 4)         │    |    │    │   │
 *    │                        │    |    │    │   │
 *    ├──                   ───┼─  ──────┼────┤   │
 *    │                   │    │    |    │    │   │
 *    │    │    │    │    │    │    |    │    │   │
 *    └────┴────┴────┴────┴────┴─  ──────┴────┘   /
 *
 * The algorithm works with a fixed number of zones \a kAwbStatsSizeX x
 * \a kAwbStatsSizeY. For example, a frame of 1296x720 is divided into 81x45
 * cells of [16x16] pixels. In the case of \a kAwbStatsSizeX=16 and
 * \a kAwbStatsSizeY=12 the zones are made of [5x4] cells. The cells are
 * left-aligned and calculated by IPAIPU3::calculateBdsGrid().
 *
 * Each statistics cell represents the average value of the pixels in that cell
 * split by colour components.
 *
 * The Accumulator structure stores the sum of the average of each cell in a
 * zone of the image, as well as the number of cells which were unsaturated and
 * therefore included in the average.
 * \todo move this description and structure into a common header
 *
 * Cells which are saturated beyond the threshold defined in
 * ipu3_uapi_awb_config_s are not included in the average.
 *
 * \var Accumulator::counted
 * \brief Number of unsaturated cells used to calculate the sums
 *
 * \var Accumulator::sum::red
 * \brief Sum of the average red values of each unsaturated cell in the zone
 *
 * \var Accumulator::sum::green
 * \brief Sum of the average green values of each unsaturated cell in the zone
 *
 * \var Accumulator::sum::blue
 * \brief Sum of the average blue values of each unsaturated cell in the zone
 */

/**
 * \struct AwbStatus
 * \brief AWB parameters calculated
 *
 * The AwbStatus structure is intended to store the AWB
 * parameters calculated by the algorithm
 *
 * \var AwbStatus::temperatureK
 * \brief Color temperature calculated
 *
 * \var AwbStatus::redGain
 * \brief Gain calculated for the red channel
 *
 * \var AwbStatus::greenGain
 * \brief Gain calculated for the green channel
 *
 * \var AwbStatus::blueGain
 * \brief Gain calculated for the blue channel
 */

/**
 * \struct Ipu3AwbCell
 * \brief Memory layout for each cell in AWB metadata
 *
 * The Ipu3AwbCell structure is used to get individual values
 * such as red average or saturation ratio in a particular cell.
 *
 * \var Ipu3AwbCell::greenRedAvg
 * \brief Green average for red lines in the cell
 *
 * \var Ipu3AwbCell::redAvg
 * \brief Red average in the cell
 *
 * \var Ipu3AwbCell::blueAvg
 * \brief blue average in the cell
 *
 * \var Ipu3AwbCell::greenBlueAvg
 * \brief Green average for blue lines
 *
 * \var Ipu3AwbCell::satRatio
 * \brief Saturation ratio in the cell
 *
 * \var Ipu3AwbCell::padding
 * \brief array of unused bytes for padding
 */

/* Default settings for Bayer noise reduction replicated from the Kernel */
static const struct ipu3_uapi_bnr_static_config imguCssBnrDefaults = {
	.wb_gains = { 16, 16, 16, 16 },
	.wb_gains_thr = { 255, 255, 255, 255 },
	.thr_coeffs = { 1700, 0, 31, 31, 0, 16 },
	.thr_ctrl_shd = { 26, 26, 26, 26 },
	.opt_center = { -648, 0, -366, 0 },
	.lut = {
		{ 17, 23, 28, 32, 36, 39, 42, 45,
		  48, 51, 53, 55, 58, 60, 62, 64,
		  66, 68, 70, 72, 73, 75, 77, 78,
		  80, 82, 83, 85, 86, 88, 89, 90 } },
	.bp_ctrl = { 20, 0, 1, 40, 0, 6, 0, 6, 0 },
	.dn_detect_ctrl = { 9, 3, 4, 0, 8, 0, 1, 1, 1, 1, 0 },
	.column_size = 1296,
	.opt_center_sqr = { 419904, 133956 },
};

/* Default color correction matrix defined as an identity matrix */
static const struct ipu3_uapi_ccm_mat_config imguCssCcmDefault = {
	8191, 0, 0, 0,
	0, 8191, 0, 0,
	0, 0, 8191, 0
};

Awb::Awb()
	: Algorithm()
{
	asyncResults_.blueGain = 1.0;
	asyncResults_.greenGain = 1.0;
	asyncResults_.redGain = 1.0;
	asyncResults_.temperatureK = 4500;

	zones_.reserve(kAwbStatsSizeX * kAwbStatsSizeY);
}

Awb::~Awb() = default;

/**
 * The function estimates the correlated color temperature using
 * from RGB color space input.
 * In physics and color science, the Planckian locus or black body locus is
 * the path or locus that the color of an incandescent black body would take
 * in a particular chromaticity space as the blackbody temperature changes.
 *
 * If a narrow range of color temperatures is considered (those encapsulating
 * daylight being the most practical case) one can approximate the Planckian
 * locus in order to calculate the CCT in terms of chromaticity coordinates.
 *
 * More detailed information can be found in:
 * https://en.wikipedia.org/wiki/Color_temperature#Approximation
 */
uint32_t Awb::estimateCCT(double red, double green, double blue)
{
	/* Convert the RGB values to CIE tristimulus values (XYZ) */
	double X = (-0.14282) * (red) + (1.54924) * (green) + (-0.95641) * (blue);
	double Y = (-0.32466) * (red) + (1.57837) * (green) + (-0.73191) * (blue);
	double Z = (-0.68202) * (red) + (0.77073) * (green) + (0.56332) * (blue);

	/* Calculate the normalized chromaticity values */
	double x = X / (X + Y + Z);
	double y = Y / (X + Y + Z);

	/* Calculate CCT */
	double n = (x - 0.3320) / (0.1858 - y);
	return 449 * n * n * n + 3525 * n * n + 6823.3 * n + 5520.33;
}

/* Generate an RGB vector with the average values for each zone */
void Awb::generateZones(std::vector<RGB> &zones)
{
	for (unsigned int i = 0; i < kAwbStatsSizeX * kAwbStatsSizeY; i++) {
		RGB zone;
		double counted = awbStats_[i].counted;
		if (counted >= kMinZonesCounted) {
			zone.G = awbStats_[i].sum.green / counted;
			if (zone.G >= kMinGreenLevelInZone) {
				zone.R = awbStats_[i].sum.red / counted;
				zone.B = awbStats_[i].sum.blue / counted;
				zones.push_back(zone);
			}
		}
	}
}

/* Translate the IPU3 statistics into the default statistics zone array */
void Awb::generateAwbStats(const ipu3_uapi_stats_3a *stats,
			   const ipu3_uapi_grid_config &grid)
{
	uint32_t cellsPerZoneX = round(grid.width / static_cast<double>(kAwbStatsSizeX));
	uint32_t cellsPerZoneY = round(grid.height / static_cast<double>(kAwbStatsSizeY));

	/*
	 * Generate a (kAwbStatsSizeX x kAwbStatsSizeY) array from the IPU3 grid which is
	 * (grid.width x grid.height).
	 */
	for (unsigned int cellY = 0; cellY < kAwbStatsSizeY * cellsPerZoneY; cellY++) {
		for (unsigned int cellX = 0; cellX < kAwbStatsSizeX * cellsPerZoneX; cellX++) {
			uint32_t cellPosition = cellY * grid.width + cellX;
			uint32_t zoneX = cellX / cellsPerZoneX;
			uint32_t zoneY = cellY / cellsPerZoneY;

			uint32_t awbZonePosition = zoneY * kAwbStatsSizeX + zoneX;

			/* Cast the initial IPU3 structure to simplify the reading */
			const ipu3_uapi_awb_set_item *currentCell = &stats->awb_raw_buffer.meta_data[cellPosition];
			if (currentCell->sat_ratio == 0) {
				/* The cell is not saturated, use the current cell */
				awbStats_[awbZonePosition].counted++;
				uint32_t greenValue = currentCell->Gr_avg + currentCell->Gb_avg;
				awbStats_[awbZonePosition].sum.green += greenValue / 2;
				awbStats_[awbZonePosition].sum.red += currentCell->R_avg;
				awbStats_[awbZonePosition].sum.blue += currentCell->B_avg;
			}
		}
	}
}

void Awb::clearAwbStats()
{
	for (unsigned int i = 0; i < kAwbStatsSizeX * kAwbStatsSizeY; i++) {
		awbStats_[i].sum.blue = 0;
		awbStats_[i].sum.red = 0;
		awbStats_[i].sum.green = 0;
		awbStats_[i].counted = 0;
	}
}

void Awb::awbGreyWorld()
{
	LOG(IPU3Awb, Debug) << "Grey world AWB";
	/*
	 * Make a separate list of the derivatives for each of red and blue, so
	 * that we can sort them to exclude the extreme gains. We could
	 * consider some variations, such as normalising all the zones first, or
	 * doing an L2 average etc.
	 */
	std::vector<RGB> &redDerivative(zones_);
	std::vector<RGB> blueDerivative(redDerivative);
	std::sort(redDerivative.begin(), redDerivative.end(),
		  [](RGB const &a, RGB const &b) {
			  return a.G * b.R < b.G * a.R;
		  });
	std::sort(blueDerivative.begin(), blueDerivative.end(),
		  [](RGB const &a, RGB const &b) {
			  return a.G * b.B < b.G * a.B;
		  });

	/* Average the middle half of the values. */
	int discard = redDerivative.size() / 4;

	RGB sumRed(0, 0, 0);
	RGB sumBlue(0, 0, 0);
	for (auto ri = redDerivative.begin() + discard,
		  bi = blueDerivative.begin() + discard;
	     ri != redDerivative.end() - discard; ri++, bi++)
		sumRed += *ri, sumBlue += *bi;

	double redGain = sumRed.G / (sumRed.R + 1),
	       blueGain = sumBlue.G / (sumBlue.B + 1);

	/* Color temperature is not relevant in Grey world but still useful to estimate it :-) */
	asyncResults_.temperatureK = estimateCCT(sumRed.R, sumRed.G, sumBlue.B);
	asyncResults_.redGain = redGain;
	asyncResults_.greenGain = 1.0;
	asyncResults_.blueGain = blueGain;
}

void Awb::calculateWBGains(const ipu3_uapi_stats_3a *stats,
			   const ipu3_uapi_grid_config &grid)
{
	ASSERT(stats->stats_3a_status.awb_en);
	zones_.clear();
	clearAwbStats();
	generateAwbStats(stats, grid);
	generateZones(zones_);
	LOG(IPU3Awb, Debug) << "Valid zones: " << zones_.size();
	if (zones_.size() > 10) {
		awbGreyWorld();
		LOG(IPU3Awb, Debug) << "Gain found for red: " << asyncResults_.redGain
				    << " and for blue: " << asyncResults_.blueGain;
	}
}

void Awb::process(IPAContext &context, const ipu3_uapi_stats_3a *stats)
{
	calculateWBGains(stats, context.configuration.grid.bdsGrid);

	/*
	 * Gains are only recalculated if enough zones were detected.
	 * The results are cached, so if no results were calculated, we set the
	 * cached values from asyncResults_ here.
	 */
	context.frameContext.awb.gains.blue = asyncResults_.blueGain;
	context.frameContext.awb.gains.green = asyncResults_.greenGain;
	context.frameContext.awb.gains.red = asyncResults_.redGain;
}

void Awb::prepare(IPAContext &context, ipu3_uapi_params *params)
{
	params->acc_param.awb.config.rgbs_thr_gr = 8191;
	params->acc_param.awb.config.rgbs_thr_r = 8191;
	params->acc_param.awb.config.rgbs_thr_gb = 8191;
	params->acc_param.awb.config.rgbs_thr_b = IPU3_UAPI_AWB_RGBS_THR_B_INCL_SAT
					       | IPU3_UAPI_AWB_RGBS_THR_B_EN
					       | 8191;

	const ipu3_uapi_grid_config &grid = context.configuration.grid.bdsGrid;

	params->acc_param.awb.config.grid = context.configuration.grid.bdsGrid;

	/*
	 * Optical center is column start (respectively row start) of the
	 * cell of interest minus its X center (respectively Y center).
	 *
	 * For the moment use BDS as a first approximation, but it should
	 * be calculated based on Shading (SHD) parameters.
	 */
	params->acc_param.bnr = imguCssBnrDefaults;
	Size &bdsOutputSize = context.configuration.grid.bdsOutputSize;
	params->acc_param.bnr.column_size = bdsOutputSize.width;
	params->acc_param.bnr.opt_center.x_reset = grid.x_start - (bdsOutputSize.width / 2);
	params->acc_param.bnr.opt_center.y_reset = grid.y_start - (bdsOutputSize.height / 2);
	params->acc_param.bnr.opt_center_sqr.x_sqr_reset = params->acc_param.bnr.opt_center.x_reset
							* params->acc_param.bnr.opt_center.x_reset;
	params->acc_param.bnr.opt_center_sqr.y_sqr_reset = params->acc_param.bnr.opt_center.y_reset
							* params->acc_param.bnr.opt_center.y_reset;
	/*
	 * Green gains should not be touched and considered 1.
	 * Default is 16, so do not change it at all.
	 * 4096 is the value for a gain of 1.0
	 */
	params->acc_param.bnr.wb_gains.gr = 16 * context.frameContext.awb.gains.green;
	params->acc_param.bnr.wb_gains.r = 4096 * context.frameContext.awb.gains.red;
	params->acc_param.bnr.wb_gains.b = 4096 * context.frameContext.awb.gains.blue;
	params->acc_param.bnr.wb_gains.gb = 16 * context.frameContext.awb.gains.green;

	LOG(IPU3Awb, Debug) << "Color temperature estimated: " << asyncResults_.temperatureK;

	/* The CCM matrix may change when color temperature will be used */
	params->acc_param.ccm = imguCssCcmDefault;

	params->use.acc_awb = 1;
	params->use.acc_bnr = 1;
	params->use.acc_ccm = 1;
}

} /* namespace ipa::ipu3::algorithms */

} /* namespace libcamera */
