/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2026, Ideas On Board
 *
 * Table-based LSC implementation
 */

#include "lsc_table.h"

/* \todo Remove RkISP1 from libipa. */
#include "linux/rkisp1-config.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(LscTable)

namespace ipa {

/**
 * \class LscTable
 * \brief Table based LSC algorithm implementation
 *
 * Table based LSC algorithm implementation. The LSCTable class implements LSC
 * support using tabular LSC data.
 *
 * \sa LscImplementation
 */

/**
 * \brief Parse tabular LSC data
 * \param[in] sets The tuning file content
 *
 * Parse the LSC data in tabular form from the \a sets tuning data.
 *
 * \return 0 on success or a negative error number otherwise
 */
int LscTable::parseLscData(const ValueNode &sets)
{
	for (const auto &set : sets.asList()) {
		uint32_t ct = set["ct"].get<uint32_t>(0);

		if (lscData_.count(ct)) {
			LOG(LscTable, Error)
				<< "Multiple sets found for color temperature "
				<< ct;
			return -EINVAL;
		}

		lsc::Components components;
		components.r = parseTable(set, "r");
		components.gr = parseTable(set, "gr");
		components.gb = parseTable(set, "gb");
		components.b = parseTable(set, "b");

		if (components.r.empty() || components.gr.empty() ||
		    components.gb.empty() || components.b.empty()) {
			LOG(LscTable, Error)
				<< "Set for color temperature " << ct
				<< " is missing tables";
			return -EINVAL;
		}

		lscData_.emplace(ct, std::move(components));
	}

	if (lscData_.empty()) {
		LOG(LscTable, Error) << "Failed to load any sets";
		return -EINVAL;
	}

	return 0;
}

std::vector<uint16_t> LscTable::parseTable(const ValueNode &tuningData,
					   const char *prop)
{
	static constexpr unsigned int kLscNumSamples =
		RKISP1_CIF_ISP_LSC_SAMPLES_MAX * RKISP1_CIF_ISP_LSC_SAMPLES_MAX;

	std::vector<uint16_t> table =
		tuningData[prop].get<std::vector<uint16_t>>().value_or(utils::defopt);
	if (table.size() != kLscNumSamples) {
		LOG(LscTable, Error)
			<< "Invalid '" << prop << "' values: expected "
			<< kLscNumSamples
			<< " elements, got " << table.size();
		return {};
	}

	return table;
}

} /* namespace ipa */

} /* namespace libcamera */
