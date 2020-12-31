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

/*
class ConfigurationSax {
	// called when null is parsed
	bool null();

	// called when a boolean is parsed; value is passed
	bool boolean(bool val);

	// called when a signed or unsigned integer number is parsed; value is passed
	bool number_integer(int val);
	bool number_unsigned(unsigned int val);

	// called when a floating-point number is parsed; value and original string is passed
	bool number_float(float val, const std::string& s);

	// called when a string is parsed; value is passed and can be safely moved away
	bool string(std::string& val);
	// called when a binary value is parsed; value is passed and can be safely moved away
	bool binary(binary& val);

	// called when an object or array begins or ends, resp. The number of elements is passed (or -1 if not known)
	bool start_object(std::size_t elements);
	bool end_object();
	bool start_array(std::size_t elements);
	bool end_array();
	// called when an object key is parsed; value is passed and can be safely moved away
	bool key(std::string& val);

	// called when a parse error occurs; byte position, the last token, and an exception is passed
	bool parse_error(std::size_t position, const std::string& last_token, const json::exception& ex);
}
*/

} /* namespace libcamera */

#endif /* __LIBCAMERA_INTERNAL_CONFIGURATION_H__ */

