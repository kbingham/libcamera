/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * json.cpp - JSON data parsing
 */
#include "libcamera/internal/json.h"

#include <iostream>
#include <stdlib.h>

#include "libcamera/internal/file.h"
#include "libcamera/internal/log.h"
#include "libcamera/internal/utils.h"

/* https://nlohmann.github.io/json/home/exceptions/#switch-off-exceptions */
#define JSON_NOEXCEPTION 1
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * \file json.h
 * \brief internal interface for parsing json data files
 */

namespace libcamera {

LOG_DEFINE_CATEGORY(JSON)

/**
 * \brief Open and parse a configuration file.
 *
 * The filename will be searched for on the libcamera configuration and paths,
 * and then parsed.
 *
 * Successfully parsed files will present the data contained therein through the
 * json object exposed from data();
 */
int Json::open(const std::string &filename)
{
	LOG(JSON, Debug) << "Reading configuration from " << filename;

	/* todo: Filename is assumed to exist here,
	 * Look into how to validate the ifstream to return error if it can't
	 * parse without throwing exceptions
	 */

	/* Parse with no error callbacks and exceptions disabled. */
	std::ifstream input(filename);
	json j = json::parse(input, nullptr, false);
	if (j.is_discarded()) {
		LOG(JSON, Error)
			<< "file: \"" << filename
			<< "\" was not parsable.";
		return -EINVAL;
	}

	json_ = std::move(j);

	return 0;
}

} /* namespace libcamera */
