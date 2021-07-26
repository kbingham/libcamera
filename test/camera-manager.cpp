/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * libcamera Camera Manager API tests
 */

#include <iostream>
#include <memory>

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>

#include "test.h"

using namespace libcamera;
using namespace std;

class CameraManagerTest : public Test
{
protected:
	int validate()
	{
		std::shared_ptr<Camera> camera;

		if (cm_->start()) {
			cerr << "Failed to start camera manager" << endl;
			return TestFail;
		}

		if (cm_->cameras().size() <= 0) {
			cerr << "No cameras available" << endl;
			return TestFail;
		}

		camera = cm_->cameras()[0];
		if (!camera) {
			cerr << "Can not obtain a camera at index 0" << endl;
			return TestFail;
		}

		/* Store the camera id that we get */
		cameraId_ = camera->id();

		return TestPass;
	}

	int run()
	{
		std::string firstCamera;

		/* Construct and validate the CameraManager */
		cm_ = new CameraManager();
		if (validate()) {
			cerr << "Failed first construction" << endl;
			return TestFail;
		}

		/* Get camera ID stored by validate */
		firstCamera = cameraId_;

		/* Now stop everything and reconstruct the CameraManager */
		cm_->stop();
		delete cm_;

		/* Restart and assert we can still get a camera */
		cm_ = new CameraManager();
		if (validate()) {
			cerr << "Failed after re-construction" << endl;
			return TestFail;
		}

		if (firstCamera != cameraId_) {
			cerr << "Expected to get the same camera after re-construction" << endl;
			return TestFail;
		}

		/* Test stop and start (without re-create) */
		cm_->stop();

		/* validate will call start() */
		if (validate()) {
			cerr << "Failed after re-starting CameraManager" << endl;
			return TestFail;
		}

		/*
		 * Creating a second camera manager is not permitted
		 *
		 * This will fail with a FATAL in constructing a second IPA
		 * Manager, even though we also have a FATAL in the
		 * CameraManager construction, but the CameraManager tries
		 * to construct an IPA manager, which fails before the
		 * CameraManager executes any of it's constructor.
		 */
		//CameraManager *cm2 = new CameraManager();

		return TestPass;
	}

private:
	CameraManager *cm_;
	std::string cameraId_;
};

TEST_REGISTER(CameraManagerTest)
