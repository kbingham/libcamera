/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2026 Ideas on Board Oy
 *
 * Base classes and types for LSC algorithm implementations
 */

#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include <libcamera/base/span.h>

#include <libcamera/geometry.h>

#include "libcamera/internal/value_node.h"

namespace libcamera {

namespace ipa {

namespace lsc {

struct Components {
	std::vector<uint16_t> r;
	std::vector<uint16_t> gr;
	std::vector<uint16_t> gb;
	std::vector<uint16_t> b;
};

using ComponentsMap = std::map<unsigned int, Components>;

} /* namespace lsc */

class LscImplementation
{
public:
	virtual ~LscImplementation() {}

	virtual int parseLscData(const ValueNode &tuningData) = 0;

	virtual lsc::ComponentsMap
	sampleForCrop(const Rectangle &cropRectangle,
		      Span<const double> xSizes,
		      Span<const double> ySizes) = 0;
};

} /* namespace ipa */

} /* namespace libcamera */
