/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Ideas On Board
 *
 * ipu3_agc.cpp - AGC/AEC control algorithm
 */

#include "ipu3_agc.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "libcamera/internal/log.h"

#include "libipa/histogram.h"

namespace libcamera {

namespace ipa {

LOG_DEFINE_CATEGORY(IPU3Agc)

/* Number of frames to wait before calculating stats on minimum exposure */
static const uint32_t kInitialFrameMinAECount = 6;
/* Number of frames to wait before calculating stats on maximum exposure */
static const uint32_t kInitialFrameMaxAECount = 12;
/* Number of frames to wait before calculating stats and estimate gain/exposure */
static const uint32_t kInitialFrameSkipCount = 18;
/* Number of frames to wait between new gain/exposure estimations */
static const uint32_t kFrameSkipCount = 6;

/* Maximum ISO value for analogue gain */
static const uint32_t kMinISO = 100;
static const uint32_t kMaxISO = 1500;
/* Maximum analogue gain value
 * \todo grab it from a camera helper */
static const uint32_t kMinGain = kMinISO / 100;
static const uint32_t kMaxGain = kMaxISO / 100;
/* \todo use calculated value based on sensor */
static const uint32_t kMinExposure = 1;
static const uint32_t kMaxExposure = 1976;

IPU3Agc::IPU3Agc()
	: frameCount_(0), lastFrame_(0),
	  converged_(false), updateControls_(false)
{
	prevExposure_ = kMinExposure;
	currentExposure_ = kMinExposure;
	nextExposure_ = kMinExposure;
	prevGain_ = kMinGain;
	currentGain_ = kMinGain;
	nextGain_ = kMinGain;
	iqMean_ = 0.0;
	prevIqMean_ = 0.0;
	currentIqMean_ = 0.0;
	nextIqMean_ = 0.0;
	spread_ = 0.0;
	median_ = 0.0;
	gamma_ = 1.0;
	histLow_ = 0;
	histHigh_ = 255;
}

IPU3Agc::~IPU3Agc()
{
}

void IPU3Agc::processBrightness(Rectangle roi, const ipu3_uapi_stats_3a *stats)
{
	Point topleft = roi.topLeft();
	uint32_t startY = (topleft.y / 16) * 129 * 8;
	uint32_t startX = (topleft.x / 8) * 8;
	uint32_t endX = (startX + (roi.size().width / 8)) * 8;

	cellsBrightness_.clear();

	uint32_t count = 0;
	for (uint32_t j = (topleft.y / 16); j < (topleft.y / 16) + (roi.size().height / 16); j++) {
		for (uint32_t i = startX + startY; i < endX + startY; i += 8) {
			uint8_t Gr = stats->awb_raw_buffer.meta_data[i + j * 129];
			uint8_t R = stats->awb_raw_buffer.meta_data[i + 1 + j * 129];
			uint8_t B = stats->awb_raw_buffer.meta_data[i + 2 + j * 129];
			uint8_t Gb = stats->awb_raw_buffer.meta_data[i + 3 + j * 129];

			/* Use the gamma encoded luma from BT.709-1 */
			cellsBrightness_.push_back(static_cast<uint32_t>(0.2125 * R + 0.7154 * (Gr + Gb) / 2 + 0.0722 * B));
			count++;
		}
	}
	std::vector<uint32_t>::iterator maxIntensity = std::max_element(cellsBrightness_.begin(), cellsBrightness_.end());
	LOG(IPU3Agc, Debug) << "Most frequent intensity is " << *maxIntensity << " at " << std::distance(cellsBrightness_.begin(), maxIntensity);

	/* \todo create a class to generate histograms ! */
	uint32_t hist[256] = { 0 };
	for (uint32_t const &val : cellsBrightness_)
		hist[val]++;

	double mean = 0.0;
	for (uint32_t i = 0; i < 256; i++) {
		mean += hist[i] * i;
	}
	mean /= count;

	double variance = 0.0;
	for (uint32_t i = 0; i < 256; i++) {
		variance += ((i - mean) * (i - mean)) * hist[i];
	}
	variance /= count;
	variance = std::sqrt(variance);

	LOG(IPU3Agc, Debug) << "mean value is: " << mean << " and variance is " << variance;
	double D = (mean + 2 * variance) - (mean - 2 * variance);

	if (D >= 256 / 3) {
		gamma_ = std::exp((1 - (mean / 255.0 + variance / 255.0)) / 2);
		LOG(IPU3Agc, Debug) << "D is " << D << " and gamma calculate: " << gamma_;
		/* Q2 case */
		if ((mean < 128) && ((mean + variance) <= 256)) {
			LOG(IPU3Agc, Debug) << "Image is dark";
		} else
			LOG(IPU3Agc, Debug) << "Image is correctly contrasted";
	} else {
		LOG(IPU3Agc, Debug) << "D is " << D << " and image is under exposed";
		gamma_ = 1.0;
	}

	/* TO CHANGE */
	gamma_ = 1.1;

	const auto [minBrightness, maxBrightness] = std::minmax_element(cellsBrightness_.begin(), cellsBrightness_.end());
	histLow_ = *minBrightness;
	histHigh_ = *maxBrightness;

	Histogram histogram(hist, 256);
	iqMean_ = histogram.interQuantileMean(0.25, 0.75);
	spread_ = histogram.quantile(0.75) - histogram.quantile(0.25);
	median_ = histogram.quantile(0.50);
	LOG(IPU3Agc, Debug) << "inter quantile mean: " << iqMean_
			    << " first: " << histogram.quantile(0.25)
			    << " last: " << histogram.quantile(0.75)
			    << " target gain: " << (0.9 * 256) / iqMean_
			    << " Q3-Q1: " << spread_
			    << " Q1: " << histogram.quantile(0.25)
			    << " Q2: " << histogram.quantile(0.50)
			    << " Q3: " << histogram.quantile(0.75)
			    << " Q4: " << histogram.quantile(1.0);
}

/* \todo make this function a math one ? */
uint32_t IPU3Agc::rootApproximation(uint32_t currentValue, uint32_t prevValue, double currentMean, double prevMean)
{
	uint32_t newValue = static_cast<uint32_t>((currentValue * prevMean + prevValue * currentMean) / (prevMean + currentMean));
	LOG(IPU3Agc, Debug) << "current: " << currentValue << " previous: " << prevValue << " current mean: " << currentMean << " previous mean: " << prevMean
			    << " new value: " << newValue;
	return newValue;
}

void IPU3Agc::lockExposureGain(uint32_t &exposure, uint32_t &gain)
{
	updateControls_ = false;

	/* Algorithm initialization wait for first valid frames */
	/* \todo - have a number of frames given by DelayedControls ?
	 * - implement a function for IIR */
	if (frameCount_ == kInitialFrameMinAECount) {
		exposure = kMinExposure;
		gain = kMinGain;

		prevExposure_ = exposure;
		prevGain_ = gain;

		converged_ = false;
		updateControls_ = true;
	} else if (frameCount_ == kInitialFrameMaxAECount) {
		prevIqMean_ = iqMean_;

		exposure = kMaxExposure;
		gain = kMaxGain;
		currentExposure_ = exposure;
		currentGain_ = gain;

		updateControls_ = true;
	} else if (frameCount_ == kInitialFrameSkipCount) {
		currentIqMean_ = iqMean_;

		exposure = std::clamp(rootApproximation(currentExposure_, prevExposure_, currentIqMean_, prevIqMean_),
				      kMinExposure, kMaxExposure);
		gain = std::clamp(rootApproximation(currentGain_, prevGain_, currentIqMean_, prevIqMean_),
				  kMinGain, kMaxGain);
		prevIqMean_ = currentIqMean_;
		nextExposure_ = exposure;
		nextGain_ = gain;

		updateControls_ = true;
		lastFrame_ = frameCount_;
	} else if ((frameCount_ > kInitialFrameSkipCount) && (frameCount_ - lastFrame_ >= kFrameSkipCount)) {
		nextIqMean_ = iqMean_;

		/* Are we good ? */
		if (std::abs(currentIqMean_ - prevIqMean_) < 5) {
			converged_ = true;
		} else {
			/* Over exposed */
			if ((currentIqMean_ - 128) > 0) {
				converged_ = false;
				LOG(IPU3Agc, Debug) << "!!! Over exposed";
				currentExposure_ = nextExposure_;
				currentGain_ = nextGain_;

				exposure = std::clamp(rootApproximation(currentExposure_, prevExposure_, currentIqMean_, prevIqMean_),
						      kMinExposure, kMaxExposure);
				gain = std::clamp(rootApproximation(currentGain_, prevGain_, currentIqMean_, prevIqMean_),
						  kMinGain, kMaxGain);
			} else {
				converged_ = false;
				LOG(IPU3Agc, Debug) << "!!! Under exposed";
				prevExposure_ = nextExposure_;
				prevGain_ = nextGain_;

				exposure = std::clamp(rootApproximation(currentExposure_, prevExposure_, currentIqMean_, prevIqMean_),
						      kMinExposure, kMaxExposure);
				gain = std::clamp(rootApproximation(currentGain_, prevGain_, currentIqMean_, prevIqMean_),
						  kMinGain, kMaxGain);
			}
			nextExposure_ = exposure;
			nextGain_ = gain;

			updateControls_ = true;
		}
		prevIqMean_ = currentIqMean_;
		currentIqMean_ = iqMean_;
		lastFrame_ = frameCount_;
	} else {
		updateControls_ = false;
	}
}

void IPU3Agc::process(const ipu3_uapi_stats_3a *stats, uint32_t &exposure, uint32_t &gain)
{
	/* \todo needs to depend on BDS !! */
	processBrightness(Rectangle(1280 / 4, 720 / 4, 1280 / 2, 720 / 2), stats);
	lockExposureGain(exposure, gain);

	LOG(IPU3Agc, Debug) << "update controls: " << updateControls_;
	frameCount_++;
}

} /* namespace ipa */

} /* namespace libcamera */
