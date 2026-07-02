/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2026 Ideas on Board Oy
 *
 * libIPA Lsc algorithm
 */

#pragma once

#include <memory>
#include <vector>

#include <libcamera/controls.h>
#include <libcamera/geometry.h>

#include "libcamera/internal/value_node.h"

#include "interpolator.h"
#include "lsc_base.h"

namespace libcamera {

namespace ipa {

namespace lsc {

struct ActiveState {
	bool enabled;
};

struct FrameContext {
	bool enabled;
	bool update;
};

} /* namespace lsc */

class LscAlgorithm
{
public:
	int init(const ValueNode &tuningData, ControlInfoMap::Map &controls,
		 const LscDescriptor &descriptor);

	int configure(lsc::ActiveState &state, const Rectangle &analogCrop,
		      const std::vector<double> &xPos,
		      const std::vector<double> &yPos);

	void queueRequest(lsc::ActiveState &state, lsc::FrameContext &context,
			  const ControlList &controls);
	void process(lsc::FrameContext &context, ControlList &metadata);

	const lsc::Components interpolateComponents(unsigned int ct)
	{
		return sets_.getInterpolated(ct);
	}

	const lsc::ComponentsMap getComponents()
	{
		return lscData_;
	}

private:
	std::unique_ptr<LscImplementation> impl_;
	Interpolator<lsc::Components> sets_;
	lsc::ComponentsMap lscData_;
	bool polynomial_;
};

} /* namespace ipa */

} /* namespace libcamera */
