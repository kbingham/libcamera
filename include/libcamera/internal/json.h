/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * json.h - JSON data parsing
 */
#ifndef __LIBCAMERA_INTERNAL_JSON_H__
#define __LIBCAMERA_INTERNAL_JSON_H__

#include <fstream>
#include <string>

/* https://nlohmann.github.io/json/home/exceptions/#switch-off-exceptions */
#define JSON_NOEXCEPTION 1
#include <nlohmann/json.hpp>

namespace libcamera {

class Json
{
public:
	int open(const std::string &filename);

	nlohmann::json &data() { return json_; }

private:
	nlohmann::json json_;
};

} /* namespace libcamera */

#endif /* __LIBCAMERA_INTERNAL_JSON_H__ */

