/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * main.cpp - lc-compliance - The libcamera compliance tool
 */

#include <iomanip>
#include <iostream>
#include <string.h>

#include <libcamera/libcamera.h>

#include "../cam/options.h"
#include "tests.h"

using namespace libcamera;

class Harness
{
public:
	Harness();
	~Harness();

	int exec(int argc, char **argv);

private:
	enum {
		OptCamera = 'c',
		OptHelp = 'h',
	};

	int parseOptions(int argc, char **argv);
	int init(int argc, char **argv);

	OptionsParser::Options options_;
	std::unique_ptr<CameraManager> cm_;
	std::shared_ptr<Camera> camera_;
};

Harness::Harness()
{
	cm_ = std::make_unique<CameraManager>();
}

Harness::~Harness()
{
	if (camera_) {
		camera_->release();
		camera_.reset();
	}

	cm_->stop();
}

int Harness::exec(int argc, char **argv)
{
	int ret = init(argc, argv);
	if (ret)
		return ret;

	std::vector<Results> results;

	results.push_back(testSingleStream(camera_));

	for (const Results &result : results) {
		ret = result.summary();
		if (ret)
			return ret;
	}

	return 0;
}

int Harness::init(int argc, char **argv)
{
	int ret = parseOptions(argc, argv);
	if (ret < 0)
		return ret;

	ret = cm_->start();
	if (ret) {
		std::cout << "Failed to start camera manager: "
			  << strerror(-ret) << std::endl;
		return ret;
	}

	if (!options_.isSet(OptCamera)) {
		std::cout << "No camera specified, available cameras:" << std::endl;
		for (const std::shared_ptr<Camera> &cam : cm_->cameras())
			std::cout << "- " << cam.get()->id() << std::endl;
		return -ENODEV;
	}

	const std::string &cameraId = options_[OptCamera];
	camera_ = cm_->get(cameraId);
	if (!camera_) {
		std::cout << "Camera " << cameraId << " not found, available cameras:" << std::endl;
		for (const std::shared_ptr<Camera> &cam : cm_->cameras())
			std::cout << "- " << cam.get()->id() << std::endl;
		return -ENODEV;
	}

	if (camera_->acquire()) {
		std::cout << "Failed to acquire camera" << std::endl;
		return -EINVAL;
	}

	std::cout << "Using camera " << cameraId << std::endl;

	return 0;
}

int Harness::parseOptions(int argc, char **argv)
{
	OptionsParser parser;
	parser.addOption(OptCamera, OptionString,
			 "Specify which camera to operate on, by id", "camera",
			 ArgumentRequired, "camera");
	parser.addOption(OptHelp, OptionNone, "Display this help message",
			 "help");

	options_ = parser.parse(argc, argv);
	if (!options_.valid())
		return -EINVAL;

	if (options_.empty() || options_.isSet(OptHelp)) {
		parser.usage();
		return options_.empty() ? -EINVAL : -EINTR;
	}

	return 0;
}

int main(int argc, char **argv)
{
	Harness harness;
	return harness.exec(argc, argv) ? EXIT_FAILURE : 0;
}
