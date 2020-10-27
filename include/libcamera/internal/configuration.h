/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * configuration.h - Finding and parsing configuration files
 */
#ifndef __LIBCAMERA_INTERNAL_CONFIGURATION_H__
#define __LIBCAMERA_INTERNAL_CONFIGURATION_H__

#include <any>
#include <fstream>
#include <string>
#include <unordered_map>

namespace libcamera {

class Json;

using ConfigurationMap = std::unordered_map<std::string, std::any>;

class Configuration
{
public:
	Configuration();
	~Configuration();

	int open(const std::string &filename);

private:
	std::string findFile(const std::string &filename);

	Json *data_;
};

} /* namespace libcamera */

#endif /* __LIBCAMERA_INTERNAL_CONFIGURATION_H__ */

