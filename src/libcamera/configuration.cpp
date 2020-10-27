/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * configuration.cpp - Parsing configuration files
 */
#include "libcamera/internal/configuration.h"

#include <iostream>
#include <stdlib.h>

#include "libcamera/internal/file.h"
#include "libcamera/internal/json.h"
#include "libcamera/internal/log.h"
#include "libcamera/internal/utils.h"

/**
 * \file configuration.h
 * \brief Read interface for configuration files
 */

namespace libcamera {

LOG_DEFINE_CATEGORY(Configuration)

Configuration::Configuration()
	: data_(nullptr)
{
}

Configuration::~Configuration()
{
	delete data_;
}

/*
 * Configuration files can be stored in system paths, which are identified
 * through the build configuration.
 *
 * However, when running uninstalled - the source location takes precedence.
 */
std::string Configuration::findFile(const std::string &filename)
{
	static std::array<std::string, 2> searchPaths = {
		LIBCAMERA_SYSCONF_DIR,
		LIBCAMERA_DATA_DIR,
	};

	std::string root = utils::libcameraSourcePath();
	if (!root.empty()) {
		std::string configurationPath = root + "data/" + filename;

		if (File::exists(configurationPath))
			return configurationPath;
	}

	for (std::string &path : searchPaths) {
		std::string configurationPath = path + "/" + filename;
		if (File::exists(configurationPath))
			return configurationPath;
	}

	return "";
}

/**
 * \brief Open and parse a configuration file.
 *
 * The filename will be searched for on the libcamera configuration and paths,
 * and then parsed.
 *
 * Successfully parsed files will present the data contained therein through the
 * json object exposed from data();
 */
int Configuration::open(const std::string &filename)
{
	std::string name = findFile(filename);
	if (name.empty()) {
		LOG(Configuration, Warning)
			<< "file: \"" << filename
			<< "\" was not found.";
		return -ENOENT;
	}

	LOG(Configuration, Debug) << "Reading configuration from " << name;

	delete data_;
	data_ = new Json();

	/* Parse with no error callbacks and exceptions disabled. */
	int ret = data_->open(name);
	if (ret)
		return ret;

	return 0;
}

} /* namespace libcamera */
