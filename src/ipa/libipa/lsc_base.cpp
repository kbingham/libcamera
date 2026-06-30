/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2026 Ideas on Board Oy
 *
 * Base classes and types for LSC algorithm implementations
 */

#include "lsc_base.h"

/**
 * \file lsc_base.h
 * \brief Base types and definitions for LscImplementation class hierarchy
 */

namespace libcamera {

namespace ipa {

namespace lsc {

/**
 * \struct Components
 * \brief Associate colour components with a list of gains
 *
 * LSC tables are defined as a list of gain values associated to a colour
 * component.
 *
 * \var Components::r
 * \brief The list of gains for the Red colour component
 *
 * \var Components::gr
 * \brief The list of gains for the Green/Red colour component
 *
 * \var Components::gb
 * \brief The list of gains for the Green/Blue colour component
 *
 * \var Components::b
 * \brief The list of gains for the Blue colour component
 */

/**
 * \typedef ComponentsMap
 * \brief Associate a colour temperature to a LSC table
 *
 * An LSC table is generated during the tuning phase for a specific colour
 * temperature, and a tuning file usually contains LSC tables generated for
 * several different colour temperatures.
 */

} /* namespace lsc */

/**
 * \class LscImplementation
 * \brief Pure virtual base class for LSC algorithm implementations
 *
 * Defines the interface for the LSC algorithm implementation.
 */

/**
 * \fn LscImplementation::~LscImplementation
 * \brief Virtual class destructor
 */

/**
 * \fn LscImplementation::parseLscData
 * \brief Parse \a tuningData
 * \param[in] tuningData The tuning data
 *
 * \return 0 on success, a negative error number otherwise
 */

/**
 * \fn LscImplementation::sampleForCrop
 * \brief Re-sample the LSC components for \a cropRectangle
 * \param[in] cropRectangle The sensor analogue crop rectangle
 * \param[in] xSizes List of horizontal positions of the LSC grid nodes
 * \param[in] ySizes List of vertical positions of the LSC grid nodes
 *
 * LSC tables have to be re-sampled every time a new sensor configuration is
 * used, as each streaming session might use a different sensor crop rectangle
 * \a cropRectangle.
 *
 * \a cropRectangle represents the size of the frame on which the LSC tables
 * have to be re-sampled on.
 *
 * \a xSizes and \a ySizes represent the position of the grid nodes vertexes in
 * the [0, 1] interval. For example an equally spaced grid of 16 nodes will have
 * each segment of size 0.0625 and the list of nodes position will be
 * [0, 0.0625, 0.125, 0.1875, ... , 1]. It is expected that the first position
 * is 0 and the last position is 1.
 */

} /* namespace ipa */

} /* namespace libcamera */
