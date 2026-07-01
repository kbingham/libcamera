/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Ideas On Board
 *
 * Polynomial based lens shading correction
 */
#pragma once

#include <array>
#include <map>
#include <vector>

#include <libcamera/base/span.h>

#include <libcamera/geometry.h>

#include "libcamera/internal/value_node.h"

#include "lsc_base.h"

namespace libcamera {

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

class LscPolynomial : public LscImplementation
{
private:
	struct PolynomialComponents {
		lsc::Polynomial pr;
		lsc::Polynomial pgr;
		lsc::Polynomial pgb;
		lsc::Polynomial pb;
	};
	using PolynomialComponentsMap = std::map<unsigned int, PolynomialComponents>;

public:
	LscPolynomial(const Size &sensorSize)
		: sensorSize_(sensorSize)
	{
	}

	int parseLscData(const ValueNode &sets) override;

	lsc::ComponentsMap
	sampleForCrop(const Rectangle &cropRectangle,
		      Span<const double> xSizes,
		      Span<const double> ySizes) override;

private:
	std::vector<double> sizesListToPositions(Span<const double> sizes);
	std::vector<uint16_t> samplePolynomial(const lsc::Polynomial &poly,
					       Span<const double> xPositions,
					       Span<const double> yPositions,
					       const Rectangle &cropRectangle);
	PolynomialComponentsMap lscData_;
	Size sensorSize_;
};

} /* namespace ipa */

} /* namespace libcamera */
