/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024 Ideas on Board Oy
 *
 agc_mean_luminance.h - Base class for mean luminance AGC algorithms
 */

#pragma once

#include <map>
#include <memory>
#include <vector>

#include <libcamera/base/utils.h>

#include <libcamera/controls.h>

#include "libcamera/internal/value_node.h"

#include "exposure_mode_helper.h"
#include "histogram.h"
#include "pwl.h"

namespace libcamera {

namespace ipa {

class AgcMeanLuminance
{
public:
	AgcMeanLuminance();
	~AgcMeanLuminance();

	struct AgcConstraint {
		enum class Bound {
			Lower = 0,
			Upper = 1
		};
		Bound bound;
		double qLo;
		double qHi;
		Pwl yTarget;
	};

	struct Traits {
		virtual ~Traits() = default;
		virtual double estimateLuminance(double gain) const = 0;
	};

	void configure(utils::Duration lineDuration, const CameraSensorHelper *sensorHelper);
	int parseTuningData(const ValueNode &tuningData);

	void setExposureCompensation(double gain)
	{
		exposureCompensation_ = gain;
	}

	void setLux(unsigned int lux)
	{
		lux_ = lux;
	}

	void setLimits(utils::Duration minExposureTime, utils::Duration maxExposureTime,
		       double minGain, double maxGain, std::vector<AgcConstraint> constraints);

	const std::map<int32_t, std::vector<AgcConstraint>> &constraintModes() const
	{
		return constraintModes_;
	}

	const std::map<int32_t, ExposureModeHelper> &exposureModeHelpers() const
	{
		return exposureModeHelpers_;
	}

	ControlInfoMap::Map controls()
	{
		return controls_;
	}

	struct Params {
		const Traits &traits;
		const Histogram &yHist;
		utils::Duration effectiveExposureValue;
		uint32_t constraintModeIndex;
		uint32_t exposureModeIndex;
	};

	struct Result : ExposureModeHelper::Result {
	};

	[[nodiscard]] Result calculateNewEv(const Params &params);

	double effectiveYTarget() const;

	void resetFrameCount()
	{
		frameCount_ = 0;
	}

private:
	int parseRelativeLuminanceTarget(const ValueNode &tuningData);
	int parseConstraint(const ValueNode &modeDict, int32_t id);
	int parseConstraintModes(const ValueNode &tuningData);
	int parseExposureModes(const ValueNode &tuningData);
	double estimateInitialGain(const Traits &traits) const;
	double constraintClampGain(uint32_t constraintModeIndex,
				   const Histogram &hist,
				   double gain) const;
	utils::Duration filterExposure(utils::Duration exposureValue);

	utils::Duration filteredExposure_;
	mutable bool luxWarningEnabled_;
	double exposureCompensation_;
	Pwl relativeLuminanceTarget_;
	uint64_t frameCount_;
	unsigned int lux_;

	std::vector<AgcConstraint> additionalConstraints_;
	std::map<int32_t, std::vector<AgcConstraint>> constraintModes_;
	std::map<int32_t, ExposureModeHelper> exposureModeHelpers_;
	ControlInfoMap::Map controls_;
};

} /* namespace ipa */

} /* namespace libcamera */
