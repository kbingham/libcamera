/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Ideas On Board
 *
 * ipu3_agc.h - IPU3 AGC/AEC control algorithm
 */
#ifndef __LIBCAMERA_IPU3_AGC_H__
#define __LIBCAMERA_IPU3_AGC_H__

#include <unordered_map>
#include <vector>

#include <linux/intel-ipu3.h>

#include <libcamera/geometry.h>

#include "libipa/algorithm.h"

namespace libcamera {

namespace ipa {

class IPU3Agc : public Algorithm
{
public:
	IPU3Agc();
	~IPU3Agc();

	void process(const ipu3_uapi_stats_3a *stats, uint32_t &exposure, uint32_t &gain);
	bool converged() { return converged_; }
	bool updateControls() { return updateControls_; }
	double gamma() { return gamma_; }

private:
	void moments(std::unordered_map<uint32_t, uint32_t> &data, int n);
	void processBrightness(Rectangle roi, const ipu3_uapi_stats_3a *stats);
	uint32_t rootApproximation(uint32_t currentValue, uint32_t prevValue, double currentMean, double prevMean);
	void lockExposureGain(uint32_t &exposure, uint32_t &gain);
	void lockExposure(uint32_t &exposure, uint32_t &gain);

	uint64_t frameCount_;
	uint64_t lastFrame_;

	/* Vector of calculated brightness for each cell */
	std::vector<uint32_t> cellsBrightness_;

	/* Values for filtering */
	uint32_t prevExposure_;
	uint32_t currentExposure_;
	uint32_t nextExposure_;
	uint32_t prevGain_;
	uint32_t currentGain_;
	uint32_t nextGain_;

	double skew_;
	double prevSkew_;
	double currentSkew_;
	bool converged_;
	bool updateControls_;

	double iqMean_;
	double prevIqMean_;
	double currentIqMean_;
	double nextIqMean_;
	double spread_;

	double median_;
	double gamma_;
	uint32_t histLow_;
	uint32_t histHigh_;
};

} /* namespace ipa */

} /* namespace libcamera */

#endif /* __LIBCAMERA_IPU3_AGC_H__ */
