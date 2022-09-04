/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Ideas On Board
 *
 * pixel.cpp - 
 */

#include "pixel.h"

namespace libcamera {

namespace ipa {

/**
 * \struct RGB
 * \brief An RGB triplet
 *
 * \var RGB::r
 * \brief The red component
 *
 * \var RGB::g
 * \brief The green component
 *
 * \var RGB::b
 * \brief The blue component
 */

/**
 * \fn RGB::RGB()
 * \brief Construct an RGB with all components set to 0
 */

/**
 * \fn RGB::RGB(T _r, T _g, T _b)
 * \brief Construct an RGB from red, green and blue values
 * \param[in] _r The red value
 * \param[in] _g The green value
 * \param[in] _b The blue value
 */

/**
 * \fn RGB::RGB(const RGB &other)
 * \brief Construct an RGB by copying \a other
 * \param[in] other The other RGB value
 */

/**
 * \fn RGB &RGB::operator=(const RGB &other)
 * \brief Replace the content of the RGB with a copy of the content of \a other
 * \param[in] other The other RGB value
 * \return This RGB value
 */

/**
 * \fn bool operator==(const RGB &lhs, const RGB &rhs)
 * \brief Compare RGB values for equality
 * \param[in] lhs The first RGB value
 * \param[in] rhs The second RGB value
 * \return True if all the components of the two RGB instances are equal, false
 * otherwise
 */

/**
 * \fn bool operator!=(const RGB &lhs, const RGB &rhs)
 * \brief Compare RGB values for inequality
 * \param[in] lhs The first RGB value
 * \param[in] rhs The second RGB value
 * \return True if any components of the two RGB instances are different, false
 * otherwise
 */

/**
 * \fn std::ostream &operator<<(std::ostream &out, const RGB &rgb)
 * \brief Insert a text representation of an RGB into an output stream
 * \param[in] out The output stream
 * \param[in] rgb The RGB value
 * \return The output stream \a out
 */

} /* namespace ipa */

} /* namespace libcamera */
