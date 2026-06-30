/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Ideas On Board
 *
 * Helper for radial polynomial used in lens shading correction.
 */
#pragma once

#include <algorithm>
#include <array>
#include <assert.h>
#include <cmath>

#include <libcamera/base/log.h>
#include <libcamera/base/span.h>

#include <libcamera/geometry.h>

#include "libcamera/internal/value_node.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(LscPolynomial)

namespace ipa {

namespace lsc {

class Polynomial
{
public:
	Polynomial(double cx = 0.0, double cy = 0.0, double k0 = 0.0,
		   double k1 = 0.0, double k2 = 0.0, double k3 = 0.0,
		   double k4 = 0.0)
		: cx_(cx), cy_(cy), cnx_(0), cny_(0),
		  coefficients_({ k0, k1, k2, k3, k4 })
	{
	}

	double sampleAtNormalizedPixelPos(double x, double y) const;
	double getM() const;
	void setReferenceImageSize(const Size &size);

private:
	double cx_;
	double cy_;
	double cnx_;
	double cny_;
	std::array<double, 5> coefficients_;
	Size imageSize_;
};

} /* namespace lsc */

} /* namespace ipa */

} /* namespace libcamera */
