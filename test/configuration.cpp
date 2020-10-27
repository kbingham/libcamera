/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * configuration.cpp - Configuration parser tests
 */

#include "libcamera/internal/configuration.h"

#include "test.h"

#include <iostream>

using namespace std;
using namespace libcamera;

class ConfigurationTest : public Test
{
protected:
	int init()
	{
		return TestPass;
	}

	int run()
	{
		Configuration c;

		/* Check access of json data before parsing */
		if (!c.data().is_null()) {
			std::cerr << "Empty json was not discarded"
				  << std::endl;
			return TestFail;
		}

		/* Failure tests */
		int ret = c.open("CantFindMe.json");
		if (ret != -ENOENT) {
			std::cerr << "Found an invalid file" << std::endl;
			return TestFail;
		}

		/*
		 * When run from source, Configuration looks in data/
		 * Find our test data relative to that.
		 */
		ret = c.open("../test/data/unparsable.json");
		if (ret != -EINVAL) {
			std::cerr << "Unparsable file parsed" << std::endl;
			return TestFail;
		}

		ret = c.open("../test/data/test_configuration.json");
		if (ret != 0) {
			std::cerr << "Failed to open configuration data"
				  << std::endl;
			return TestFail;
		}

		/* Parse sample data */
		json j = c.data();

		for (auto &array : j) {
			std::cout << array << std::endl;
		}

		for (auto &[key, value] : c.data().items()) {
			std::cout << key << " : " << value << "\n";
		}

		std::string cameraName = "Not Found";

		auto it = j.find("device");
		if (it == j.end()) {
			std::cerr << "Failed to find device key" << std::endl;
			return TestFail;
		}

		std::cout << "Device: " << j["device"] << std::endl;
		if (j["device"].contains("cameraName"))
			cameraName = j["device"]["cameraName"];

		for (auto &[key, value] : j["device"].items()) {
			std::cout << key << " : " << value << "\n";
		}

		if (cameraName != "Test Camera Name") {
			std::cerr << "Failed to find expected string" << std::endl;
			return TestFail;
		}

		return TestPass;
	}

	void cleanup()
	{
	}

private:
	std::string fileName_;
};

TEST_REGISTER(ConfigurationTest)

