/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * configuration.cpp - Configuration parser tests
 */

#include "libcamera/internal/configuration.h"

#include "test.h"

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
		return TestPass;
	}

	void cleanup()
	{
	}

private:
	std::string fileName_;
};

TEST_REGISTER(ConfigurationTest)

