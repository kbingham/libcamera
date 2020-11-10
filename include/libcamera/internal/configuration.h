/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * configuration.h - Parsing configuration files
 */
#ifndef __LIBCAMERA_INTERNAL_CONFIGURATION_H__
#define __LIBCAMERA_INTERNAL_CONFIGURATION_H__

#include <fstream>
#include <string>

/* https://nlohmann.github.io/json/home/exceptions/#switch-off-exceptions */
#define JSON_NOEXCEPTION 1
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace libcamera {

class Configuration
{
public:
	Configuration();

	int open(std::string filename);

	json &data() { return json_; }

private:
	std::string findFile(std::string filename);

	json json_;
};

} /* namespace libcamera */

#endif /* __LIBCAMERA_INTERNAL_CONFIGURATION_H__ */

