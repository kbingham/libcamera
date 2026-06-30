/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Ideas On Board
 *
 * Polynomial class to represent lens shading correction
 */

#include "lsc_polynomial.h"

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
 * \fn Polynomial::sampleAtNormalizedPixelPos(double x, double y)
 * \brief Sample the polynomial at the given normalized pixel position
 *
 * This functions samples the polynomial at the given pixel position divided by
 * the value returned by getM().
 *
 * \param x x position in normalized coordinates
 * \param y y position in normalized coordinates
 * \return The sampled value
 */

/**
 * \fn Polynomial::getM()
 * \brief Get the value m as described in the dng specification
 *
 * Returns m according to dng spec. m represents the Euclidean distance
 * (in pixels) from the optical center to the farthest pixel in the
 * image.
 *
 * \return The sampled value
 */

/**
 * \fn Polynomial::setReferenceImageSize(const Size &size)
 * \brief Set the reference image size
 *
 * Set the reference image size that is used for subsequent calls to getM() and
 * sampleAtNormalizedPixelPos()
 *
 * \param size The size of the reference image
 */

} /* namespace lsc */

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
