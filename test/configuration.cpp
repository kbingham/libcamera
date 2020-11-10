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

		/* Failure tests */
		int ret = c.open("CantFindMe.json");
		if (ret != -ENOENT) {
			std::cout << "Found an invalid file" << std::endl;
			return TestFail;
		}

		ret = c.open("unparsable.json");
		if (ret != -EINVAL) {
			std::cout << "Unparsable file parsed" << std::endl;
			return TestFail;
		}

		ret = c.open("configuration.json");
		if (ret == 0)
			std::cout << "Success (configuration.json)" << std::endl;

/*
		c.open("arc/features.json");
		if (ret == 0)
			std::cout << "Success (features.json)" << std::endl;


		json properties = c.data()["properties"];

		Configuration newConf;
		for (auto &[key, value] : properties.items()) {
			std::cout << key << " : " << value << "\n";
		}
*/

#if 1

		json j = c.data();

		for (auto &array : j) {
			std::cout << array << std::endl;
		}

		for (auto &[key, value] : c.data().items()) {
			std::cout << key << " : " << value << "\n";
		}

		std::cout << j["Kieran"] << std::endl;
		std::string proxy = j["proxy"];

		std::cout << "Proxy: " << proxy << std::endl;


		std::string cameraName = "Not Found";
		if (j.contains("device")) {
			std::cout << "Device: " << j["device"] << std::endl;
			if (j["device"].contains("cameraName"))
				cameraName = j["device"]["cameraName"];

			for (auto &[key, value] : j["device"].items()) {
				std::cout << key << " : " << value << "\n";
			}
		}

		std::cout << "Camera Name: " << cameraName << std::endl;
#endif
		return TestPass;
	}

	void cleanup()
	{
	}

private:
	std::string fileName_;
};

TEST_REGISTER(ConfigurationTest)

