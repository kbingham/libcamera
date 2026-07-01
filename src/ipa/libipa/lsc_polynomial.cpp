/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Ideas On Board
 *
 * Polynomial based lens shading correction
 */

#include "lsc_polynomial.h"

#include <assert.h>
#include <cmath>

#include <libcamera/base/log.h>

/**
 * \file lsc_polynomial.h
 * \brief LscPolynomial class
 */

namespace libcamera {

LOG_DEFINE_CATEGORY(LscPolynomial)

namespace ipa {

namespace lsc {

/**
 * \class Polynomial
 * \brief Class for handling even polynomials used in lens shading correction
 *
 * Shading artifacts of camera lenses can be modeled using even radial
 * polynomials. This class implements a polynomial with 5 coefficients which
 * follows the definition of the FixVignetteRadial opcode in the Adobe DNG
 * specification.
 */

/**
 * \fn Polynomial::Polynomial(double cx = 0.0, double cy = 0.0, double k0 = 0.0,
		      double k1 = 0.0, double k2 = 0.0, double k3 = 0.0,
		      double k4 = 0.0)
 * \brief Construct a polynomial using the given coefficients
 * \param cx Center-x relative to the image in normalized coordinates (0..1)
 * \param cy Center-y relative to the image in normalized coordinates (0..1)
 * \param k0 Coefficient of the polynomial
 * \param k1 Coefficient of the polynomial
 * \param k2 Coefficient of the polynomial
 * \param k3 Coefficient of the polynomial
 * \param k4 Coefficient of the polynomial
 */

/**
 * \brief Sample the polynomial at the given normalized pixel position
 *
 * This functions samples the polynomial at the given pixel position divided by
 * the value returned by getM().
 *
 * \param x x position in normalized coordinates
 * \param y y position in normalized coordinates
 * \return The sampled value
 */
double Polynomial::sampleAtNormalizedPixelPos(double x, double y) const
{
	double dx = x - cnx_;
	double dy = y - cny_;
	double r = sqrt(dx * dx + dy * dy);
	double res = 1.0;

	for (unsigned int i = 0; i < coefficients_.size(); i++)
		res += coefficients_[i] * std::pow(r, (i + 1) * 2);

	return res;
}

/**
 * \brief Get the value m as described in the dng specification
 *
 * Returns m according to dng spec. m represents the Euclidean distance
 * (in pixels) from the optical center to the farthest pixel in the
 * image.
 *
 * \return The sampled value
 */
double Polynomial::getM() const
{
	double cpx = imageSize_.width * cx_;
	double cpy = imageSize_.height * cy_;
	double mx = std::max(cpx, std::fabs(imageSize_.width - cpx));
	double my = std::max(cpy, std::fabs(imageSize_.height - cpy));

	return sqrt(mx * mx + my * my);
}

/**
 * \brief Set the reference image size
 *
 * Set the reference image size that is used for subsequent calls to getM() and
 * sampleAtNormalizedPixelPos()
 *
 * \param size The size of the reference image
 */
void Polynomial::setReferenceImageSize(const Size &size)
{
	assert(!size.isNull());
	imageSize_ = size;

	/* Calculate normalized centers */
	double m = getM();
	cnx_ = (size.width * cx_) / m;
	cny_ = (size.height * cy_) / m;
}

} /* namespace lsc */

/**
 * \class LscPolynomial
 * \brief Radial Polynomial LSC algorithm implementation
 *
 * Polynomial-based LSC algorithm implementation. The LscPolynomial class
 * implements LSC support using a Polynomial to represent the shading artifacts
 * map.
 *
 * \sa LscImplementation
 */

/**
 * \fn LscPolynomial::LscPolynomial
 * \param[in] sensorSize The physical sensor size
 *
 * Construct an LscPolynomial
 */

/**
 * \brief Parse polynomial LSC data
 * \param[in] sets The tuning file content
 *
 * Parse the LSC data in polyomial form from the \a sets tuning data.
 *
 * \return 0 on success or a negative error number otherwise
 */
int LscPolynomial::parseLscData(const ValueNode &sets)
{
	for (const auto &set : sets.asList()) {
		std::optional<lsc::Polynomial> pr, pgr, pgb, pb;
		uint32_t ct = set["ct"].get<uint32_t>(0);

		if (lscData_.count(ct)) {
			LOG(LscPolynomial, Error)
				<< "Multiple sets found for "
				<< "color temperature " << ct;
			return -EINVAL;
		}

		pr = set["r"].get<lsc::Polynomial>();
		pgr = set["gr"].get<lsc::Polynomial>();
		pgb = set["gb"].get<lsc::Polynomial>();
		pb = set["b"].get<lsc::Polynomial>();

		if (!(pr || pgr || pgb || pb)) {
			LOG(LscPolynomial, Error)
				<< "Failed to parse polynomial for "
				<< "colour temperature " << ct;
			return -EINVAL;
		}

		pr->setReferenceImageSize(sensorSize_);
		pgr->setReferenceImageSize(sensorSize_);
		pgb->setReferenceImageSize(sensorSize_);
		pb->setReferenceImageSize(sensorSize_);

		lscData_.emplace(std::piecewise_construct,
				 std::forward_as_tuple(ct),
				 std::forward_as_tuple(PolynomialComponents{ *pr, *pgr, *pgb, *pb }));
	}

	if (lscData_.empty()) {
		LOG(LscPolynomial, Error) << "Failed to load any sets";
		return -EINVAL;
	}

	return 0;
}

/**
 * \brief Re-sample the LSC components for \a cropRectangle
 * \param[in] cropRectangle The sensor analogue crop rectangle
 * \param[in] xSizes List of horizontal positions of the LSc grid nodes
 * \param[in] ySizes List of vertical positions of the LSC grid nodes
 *
 * LSC tables have to be re-sampled every time a new sensor configuration is
 * used, as each streaming session might use a different sensor crop rectangle.
 *
 * Polynomial LSC tables can be re-sampled for a given sensor frame resolution
 * using a list of horizontal and vertical nodes that define the LSC grid on
 * which the polynomial is re-sampled on.
 *
 * \a cropRectangle represents the size of the frame on which the LSC tables
 * have to be re-sampled on.
 *
 * \a xSizes and \a ySizes represent the position of the grid nodes vertexes in
 * the [0, 1] interval. In example an equally spaced grid of 16 nodes will have
 * each segment of size 0.0625 and the list of nodes position will be
 * [0, 0.0625, 0.125, 0.1875, ... , 1]. It is expected that the first position
 * is 0 and the last position is 1.
 */
lsc::ComponentsMap
LscPolynomial::sampleForCrop(const Rectangle &cropRectangle,
			     Span<const double> xSizes,
			     Span<const double> ySizes)
{
	std::vector<double> xPos = sizesListToPositions(xSizes);
	std::vector<double> yPos = sizesListToPositions(ySizes);

	lsc::ComponentsMap components;

	for (const auto &[k, p] : lscData_) {
		components[k] = {
			samplePolynomial(p.pr, xPos, yPos, cropRectangle),
			samplePolynomial(p.pgr, xPos, yPos, cropRectangle),
			samplePolynomial(p.pgb, xPos, yPos, cropRectangle),
			samplePolynomial(p.pb, xPos, yPos, cropRectangle)
		};
	}

	return components;
}

std::vector<uint16_t>
LscPolynomial::samplePolynomial(const lsc::Polynomial &poly,
				Span<const double> xPositions,
				Span<const double> yPositions,
				const Rectangle &cropRectangle)
{
	double m = poly.getM();
	double x0 = cropRectangle.x / m;
	double y0 = cropRectangle.y / m;
	double w = cropRectangle.width / m;
	double h = cropRectangle.height / m;
	std::vector<uint16_t> samples;

	samples.reserve(xPositions.size() * yPositions.size());

	for (double y : yPositions) {
		for (double x : xPositions) {
			double xp = x0 + x * w;
			double yp = y0 + y * h;
			/*
			 * The hardware uses 2.10 fixed point format and limits
			 * the legal values to [1..3.999]. Scale and clamp the
			 * sampled value accordingly.
			 */
			int v = static_cast<int>(
				poly.sampleAtNormalizedPixelPos(xp, yp) *
				1024);
			v = std::clamp(v, 1024, 4095);
			samples.push_back(v);
		}
	}
	return samples;
}

/*
 * The rkisp1 LSC grid spacing is defined by the cell sizes on the top-left
 * quadrant of the grid. This is then mirrored in hardware to the other
 * quadrants. See parseSizes() for further details. For easier handling, this
 * function converts the cell sizes of half the grid to a list of position of
 * the whole grid (on one axis). Example:
 *
 * input:   | 0.2 | 0.3 |
 * output: 0.0   0.2   0.5   0.8   1.0
 */
std::vector<double>
LscPolynomial::sizesListToPositions(Span<const double> sizes)
{
	const int half = sizes.size();
	std::vector<double> positions(half * 2 + 1);
	double x = 0.0;

	positions[half] = 0.5;
	for (int i = 1; i <= half; i++) {
		x += sizes[half - i];
		positions[half - i] = 0.5 - x;
		positions[half + i] = 0.5 + x;
	}

	return positions;
}

} /* namespace ipa */

#ifndef __DOXYGEN__
template<>
std::optional<ipa::lsc::Polynomial>
ValueNode::Accessor<ipa::lsc::Polynomial>::get(const ValueNode &obj) const
{
	std::optional<double> cx = obj["cx"].get<double>();
	std::optional<double> cy = obj["cy"].get<double>();
	std::optional<double> k0 = obj["k0"].get<double>();
	std::optional<double> k1 = obj["k1"].get<double>();
	std::optional<double> k2 = obj["k2"].get<double>();
	std::optional<double> k3 = obj["k3"].get<double>();
	std::optional<double> k4 = obj["k4"].get<double>();

	if (!(cx && cy && k0 && k1 && k2 && k3 && k4)) {
		LOG(LscPolynomial, Error)
			<< "Polynomial is missing a parameter";
		return std::nullopt;
	}

	return ipa::lsc::Polynomial(*cx, *cy, *k0, *k1, *k2, *k3, *k4);
}
#endif /* __DOXYGEN__ */

} /* namespace libcamera */
