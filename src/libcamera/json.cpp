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

/*******************************************************************************
STL-like access reference guide.

We designed the JSON class to behave just like an STL container. In fact, it
satisfies the ReversibleContainer requirement.

	// create an array using push_back
	json j;
	j.push_back("foo");
	j.push_back(1);
	j.push_back(true);

	// also use emplace_back
	j.emplace_back(1.78);

	// iterate the array
	for (json::iterator it = j.begin(); it != j.end(); ++it) {
		std::cout << *it << '\n';
	}

	// range-based for
	for (auto& element : j) {
		std::cout << element << '\n';
	}

	// getter/setter
	const auto tmp = j[0].get<std::string>();
	j[1] = 42;
	bool foo = j.at(2);

	// comparison
	j == "[\"foo\", 42, true]"_json;  // true

	// other stuff
	j.size();     // 3 entries
	j.empty();    // false
	j.type();     // json::value_t::array
	j.clear();    // the array is empty again

	// convenience type checkers
	j.is_null();
	j.is_boolean();
	j.is_number();
	j.is_object();
	j.is_array();
	j.is_string();

	// create an object
	json o;
	o["foo"] = 23;
	o["bar"] = false;
	o["baz"] = 3.141;

	// also use emplace
	o.emplace("weather", "sunny");

	// special iterator member functions for objects
	for (json::iterator it = o.begin(); it != o.end(); ++it) {
		std::cout << it.key() << " : " << it.value() << "\n";
	}

	// the same code as range for
	for (auto& el : o.items()) {
		std::cout << el.key() << " : " << el.value() << "\n";
	}

	// even easier with structured bindings (C++17)
	for (auto& [key, value] : o.items()) {
		std::cout << key << " : " << value << "\n";
	}

	// find an entry
	if (o.contains("foo")) {
		// there is an entry with key "foo"
	}

	// or via find and an iterator
	if (o.find("foo") != o.end()) {
		// there is an entry with key "foo"
	}

	// or simpler using count()
	int foo_present = o.count("foo"); // 1
	int fob_present = o.count("fob"); // 0

	// delete an entry
	o.erase("foo");
*******************************************************************************/

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
