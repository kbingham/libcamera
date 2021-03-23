/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * camera_hal_config.h - Camera HAL configuration file manager
 */
#ifndef __ANDROID_CAMERA_HAL_CONFIG_H__
#define __ANDROID_CAMERA_HAL_CONFIG_H__

#include <filesystem>
#include <map>
#include <string>

#include <libcamera/class.h>

struct CameraProps {
	int facing = -1;
	int rotation = -1;

	bool valid = false;
};

class CameraHalConfig final : public libcamera::Extensible
{
	LIBCAMERA_DECLARE_PRIVATE(CameraBuffer)

public:
	CameraHalConfig();

	bool exists() const { return exists_; }
	bool valid() const { return valid_; }

	int open();
	const CameraProps &cameraProps(const std::string &cameraId) const;

private:
	std::filesystem::path findFilePath(const std::string &filename) const;
	FILE *openConfigFile(const std::string &filename);

	bool exists_;
	bool valid_;
	std::map<std::string, CameraProps> cameras_;
};
#endif /* __ANDROID_CAMERA_HAL_CONFIG_H__ */
