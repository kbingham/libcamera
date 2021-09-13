/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Ideas On Board
 *
 * ipu3_agc.cpp - AGC/AEC control algorithm
 */

#include "agc.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include <libcamera/base/log.h>

#include <libcamera/ipa/core_ipa_interface.h>

#include "libipa/histogram.h"

namespace libcamera {

using namespace std::literals::chrono_literals;

namespace ipa::ipu3::algorithms {

LOG_DEFINE_CATEGORY(IPU3Agc)

/* Number of frames to wait before calculating stats on minimum exposure */
static constexpr uint32_t kInitialFrameMinAECount = 4;
/* Number of frames to wait between new gain/exposure estimations */
static constexpr uint32_t kFrameSkipCount = 6;

/* Maximum ISO value for analogue gain */
static constexpr uint32_t kMinISO = 100;
static constexpr uint32_t kMaxISO = 1500;

/* Maximum analogue gain value
 * \todo grab it from a camera helper */
static constexpr uint32_t kMinGain = kMinISO / 100;
static constexpr uint32_t kMaxGain = kMaxISO / 100;

/* \todo use calculated value based on sensor */
static constexpr uint32_t kMinExposure = 1;
static constexpr uint32_t kMaxExposure = 1976;

/* Histogram constants */
static constexpr uint32_t knumHistogramBins = 256;
static constexpr double kEvGainTarget = 0.5;

/* A cell is 8 bytes and contains averages for RGB values and saturation ratio */
static constexpr uint8_t kCellSize = 8;

Agc::Agc()
	: frameCount_(0), lastFrame_(0), iqMean_(0.0), lineDuration_(0s),
	  maxExposureTime_(0s), prevExposure_(0s), prevExposureNoDg_(0s),
	  currentExposure_(0s), currentExposureNoDg_(0s)
{
}

int Agc::configure([[maybe_unused]] IPAContext &context,
		   const IPAConfigInfo &configInfo)
{
	lineDuration_ = configInfo.sensorInfo.lineLength * 1.0s
		      / configInfo.sensorInfo.pixelRate;
	maxExposureTime_ = kMaxExposure * lineDuration_;

	return 0;
}

void Agc::processBrightness(const ipu3_uapi_stats_3a *stats,
			    const ipu3_uapi_grid_config &grid)
{
	const struct ipu3_uapi_grid_config statsAeGrid = stats->stats_4a_config.awb_config.grid;
	Rectangle aeRegion = { statsAeGrid.x_start,
			       statsAeGrid.y_start,
			       static_cast<unsigned int>(statsAeGrid.x_end - statsAeGrid.x_start) + 1,
			       static_cast<unsigned int>(statsAeGrid.y_end - statsAeGrid.y_start) + 1 };
	Point topleft = aeRegion.topLeft();
	int topleftX = topleft.x >> grid.block_width_log2;
	int topleftY = topleft.y >> grid.block_height_log2;

	/* Align to the grid cell width and height */
	uint32_t startX = topleftX << grid.block_width_log2;
	uint32_t startY = topleftY * grid.width << grid.block_width_log2;
	uint32_t endX = (startX + (aeRegion.size().width >> grid.block_width_log2)) << grid.block_width_log2;
	uint32_t i, j;
	uint32_t count = 0;

	uint32_t hist[knumHistogramBins] = { 0 };
	for (j = topleftY;
	     j < topleftY + (aeRegion.size().height >> grid.block_height_log2);
	     j++) {
		for (i = startX + startY; i < endX + startY; i += kCellSize) {
			/*
			 * The grid width (and maybe height) is not reliable.
			 * We observed a bit shift which makes the value 160 to be 32 in the stats grid.
			 * Use the one passed at init time.
			 */
			const ipu3_uapi_awb_set_item *currentCell = &stats->awb_raw_buffer.meta_data[i + j * grid.width];
			if (currentCell->sat_ratio == 0) {
				uint8_t Gr = currentCell->Gr_avg;
				uint8_t Gb = currentCell->Gb_avg;
				hist[(Gr + Gb) / 2]++;
				count++;
			}
		}
	}

	/* Estimate the quantile mean of the top 2% of the histogram */
	iqMean_ = Histogram(Span<uint32_t>(hist)).interQuantileMean(0.98, 1.0);
}

void Agc::filterExposure()
{
	double speed = 0.2;
	if (prevExposure_ == 0s) {
		/* DG stands for digital gain.*/
		prevExposure_ = currentExposure_;
		prevExposureNoDg_ = currentExposureNoDg_;
	} else {
		/*
		 * If we are close to the desired result, go faster to avoid making
		 * multiple micro-adjustments.
		 * \ todo: Make this customisable?
		 */
		if (prevExposure_ < 1.2 * currentExposure_ &&
		    prevExposure_ > 0.8 * currentExposure_)
			speed = sqrt(speed);

		prevExposure_ = speed * currentExposure_ +
				prevExposure_ * (1.0 - speed);
		prevExposureNoDg_ = speed * currentExposureNoDg_ +
				prevExposureNoDg_ * (1.0 - speed);
	}
	/*
	 * We can't let the no_dg exposure deviate too far below the
	 * total exposure, as there might not be enough digital gain available
	 * in the ISP to hide it (which will cause nasty oscillation).
	 */
	double fastReduceThreshold = 0.4;
	if (prevExposureNoDg_ <
	    prevExposure_ * fastReduceThreshold)
		prevExposureNoDg_ = prevExposure_ * fastReduceThreshold;
	LOG(IPU3Agc, Debug) << "After filtering, total_exposure " << prevExposure_;
}

void Agc::lockExposureGain(uint32_t &exposure, double &gain)
{
	/* Algorithm initialization should wait for first valid frames */
	/* \todo - have a number of frames given by DelayedControls ?
	 * - implement a function for IIR */
	if ((frameCount_ < kInitialFrameMinAECount) || (frameCount_ - lastFrame_ < kFrameSkipCount))
		return;

	/* Are we correctly exposed ? */
	if (std::abs(iqMean_ - kEvGainTarget * knumHistogramBins) <= 1) {
		LOG(IPU3Agc, Debug) << "!!! Good exposure with iqMean = " << iqMean_;
	} else {
		double newGain = kEvGainTarget * knumHistogramBins / iqMean_;

		/* extracted from Rpi::Agc::computeTargetExposure */
		libcamera::utils::Duration currentShutter = exposure * lineDuration_;
		currentExposureNoDg_ = currentShutter * gain;
		LOG(IPU3Agc, Debug) << "Actual total exposure " << currentExposureNoDg_
				    << " Shutter speed " << currentShutter
				    << " Gain " << gain;
		currentExposure_ = currentExposureNoDg_ * newGain;
		libcamera::utils::Duration maxTotalExposure = maxExposureTime_ * kMaxGain;
		currentExposure_ = std::min(currentExposure_, maxTotalExposure);
		LOG(IPU3Agc, Debug) << "Target total exposure " << currentExposure_;

		/* \todo: estimate if we need to desaturate */
		filterExposure();

		libcamera::utils::Duration newExposure = 0.0s;
		if (currentShutter < maxExposureTime_) {
			exposure = std::clamp(static_cast<uint32_t>(exposure * currentExposure_ / currentExposureNoDg_), kMinExposure, kMaxExposure);
			newExposure = currentExposure_ / exposure;
			gain = std::clamp(static_cast<uint32_t>(gain * currentExposure_ / newExposure), kMinGain, kMaxGain);
		} else if (currentShutter >= maxExposureTime_) {
			gain = std::clamp(static_cast<uint32_t>(gain * currentExposure_ / currentExposureNoDg_), kMinGain, kMaxGain);
			newExposure = currentExposure_ / gain;
			exposure = std::clamp(static_cast<uint32_t>(exposure * currentExposure_ / newExposure), kMinExposure, kMaxExposure);
		}
		LOG(IPU3Agc, Debug) << "Adjust exposure " << exposure * lineDuration_ << " and gain " << gain;
	}
	lastFrame_ = frameCount_;
}

void Agc::process(IPAContext &context, const ipu3_uapi_stats_3a *stats)
{
	uint32_t &exposure = context.frameContext.agc.exposure;
	double &gain = context.frameContext.agc.gain;
	processBrightness(stats, context.configuration.grid.bdsGrid);
	lockExposureGain(exposure, gain);
	frameCount_++;
}

} /* namespace ipa::ipu3::algorithms */

} /* namespace libcamera */
