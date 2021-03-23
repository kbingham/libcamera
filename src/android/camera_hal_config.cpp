/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * camera_hal_config.cpp - Camera HAL configuration file manager
 */
#include "camera_hal_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <yaml.h>

#include <hardware/camera3.h>

#include "libcamera/internal/log.h"

using namespace libcamera;

LOG_DEFINE_CATEGORY(HALConfig)

class CameraHalConfig::Private : public Extensible::Private
{
	LIBCAMERA_DECLARE_PUBLIC(CameraHalConfig)

public:
	Private(CameraHalConfig *halConfig);

	int parseConfigFile(FILE *fh, std::map<std::string, CameraProps> *cameras);

private:
	std::string parseValue();
	std::string parseKey();
	int parseValueBlock();
	int parseCameraLocation(CameraProps *cameraProps, const std::string &location);
	int parseCameraProps(const std::string &cameraId);
	int parseCameras();
	int parseEntry();

	yaml_parser_t parser_;
	std::map<std::string, CameraProps> *cameras_;
};

CameraHalConfig::Private::Private(CameraHalConfig *halConfig)
	: Extensible::Private(halConfig)
{
}

std::string CameraHalConfig::Private::parseValue()
{
	yaml_token_t token;

	/* Make sure the token type is a value and get its content. */
	yaml_parser_scan(&parser_, &token);
	if (token.type != YAML_VALUE_TOKEN) {
		yaml_token_delete(&token);
		return "";
	}
	yaml_token_delete(&token);

	yaml_parser_scan(&parser_, &token);
	if (token.type != YAML_SCALAR_TOKEN) {
		yaml_token_delete(&token);
		return "";
	}

	std::string value(reinterpret_cast<char *>(token.data.scalar.value),
			  token.data.scalar.length);
	yaml_token_delete(&token);

	return value;
}

std::string CameraHalConfig::Private::parseKey()
{
	yaml_token_t token;

	/* Make sure the token type is a key and get its value. */
	yaml_parser_scan(&parser_, &token);
	if (token.type != YAML_SCALAR_TOKEN) {
		yaml_token_delete(&token);
		return "";
	}

	std::string value(reinterpret_cast<char *>(token.data.scalar.value),
			  token.data.scalar.length);
	yaml_token_delete(&token);

	return value;
}

int CameraHalConfig::Private::parseValueBlock()
{
	yaml_token_t token;

	/* Make sure the next token are VALUE and BLOCK_MAPPING_START. */
	yaml_parser_scan(&parser_, &token);
	if (token.type != YAML_VALUE_TOKEN) {
		yaml_token_delete(&token);
		return -EINVAL;
	}
	yaml_token_delete(&token);

	yaml_parser_scan(&parser_, &token);
	if (token.type != YAML_BLOCK_MAPPING_START_TOKEN) {
		yaml_token_delete(&token);
		return -EINVAL;
	}
	yaml_token_delete(&token);

	return 0;
}

int CameraHalConfig::Private::parseCameraLocation(CameraProps *cameraProps, const std::string &location)
{
	if (location == "front")
		cameraProps->facing = CAMERA_FACING_FRONT;
	else if (location == "back")
		cameraProps->facing = CAMERA_FACING_BACK;
	else if (location == "external")
		cameraProps->facing = CAMERA_FACING_EXTERNAL;
	else
		return -EINVAL;

	return 0;
}

int CameraHalConfig::Private::parseCameraProps(const std::string &cameraId)
{
	int ret = parseValueBlock();
	if (ret)
		return ret;

	/*
	 * Parse the camera properties and store them in a cameraProps instance.
	 *
	 * Add a safety counter to make sure we don't loop indefinitely in case
	 * the configuration file is malformed.
	 */
	unsigned int sentinel = 100;
	CameraProps cameraProps;
	bool blockEnd = false;
	yaml_token_t token;

	do {
		yaml_parser_scan(&parser_, &token);
		switch (token.type) {
		case YAML_KEY_TOKEN: {
			yaml_token_delete(&token);

			/*
			 * Parse the camera property key and make sure it is
			 * valid.
			 */
			std::string key = parseKey();
			std::string value = parseValue();
			if (key.empty() || value.empty())
				return -EINVAL;

			if (key == "location") {
				ret = parseCameraLocation(&cameraProps, value);
				if (ret) {
					LOG(HALConfig, Error)
						<< "Unknown location: " << value;
					return -EINVAL;
				}
			} else if (key == "rotation") {
				ret = std::stoi(value);
				if (ret < 0 || ret >= 360) {
					LOG(HALConfig, Error)
						<< "Unknown rotation: "
						<< cameraProps.rotation;
					return -EINVAL;
				}
				cameraProps.rotation = ret;
			} else {
				LOG(HALConfig, Error)
					<< "Unknown key: " << key;
				return -EINVAL;
			}
			break;
		}

		case YAML_BLOCK_END_TOKEN:
			blockEnd = true;
			[[fallthrough]];
		default:
			yaml_token_delete(&token);
			break;
		}

		--sentinel;
	} while (!blockEnd && sentinel);
	if (!sentinel)
		return -EINVAL;

	cameraProps.valid = true;
	(*cameras_)[cameraId] = cameraProps;

	return 0;
}

int CameraHalConfig::Private::parseCameras()
{
	int ret = parseValueBlock();
	if (ret) {
		LOG(HALConfig, Error) << "Configuration file is not valid";
		return ret;
	}

	/*
	 * Parse the camera properties.
	 *
	 * Each camera properties block is a list of properties associated
	 * with the ID (as assembled by CameraSensor::generateId()) of the
	 * camera they refer to.
	 *
	 * cameras:
	 *   "camera0 id":
	 *     key: value
	 *     key: value
	 *     ...
	 *
	 *   "camera1 id":
	 *     key: value
	 *     key: value
	 *     ...
	 */
	bool blockEnd = false;
	yaml_token_t token;
	do {
		yaml_parser_scan(&parser_, &token);
		switch (token.type) {
		case YAML_KEY_TOKEN: {
			yaml_token_delete(&token);

			/* Parse the camera ID as key of the property list. */
			std::string cameraId = parseKey();
			if (cameraId.empty())
				return -EINVAL;

			ret = parseCameraProps(cameraId);
			if (ret)
				return -EINVAL;
			break;
		}
		case YAML_BLOCK_END_TOKEN:
			blockEnd = true;
			[[fallthrough]];
		default:
			yaml_token_delete(&token);
			break;
		}
	} while (!blockEnd);

	return 0;
}

int CameraHalConfig::Private::parseEntry()
{
	int ret = -EINVAL;

	/*
	 * Parse each key we find in the file.
	 *
	 * The 'cameras' keys maps to a list of (lists) of camera properties.
	 */

	std::string key = parseKey();
	if (key.empty())
		return ret;

	if (key == "cameras")
		ret = parseCameras();
	else
		LOG(HALConfig, Error) << "Unknown key: " << key;

	return ret;
}

int CameraHalConfig::Private::parseConfigFile(FILE *fh,
					      std::map<std::string, CameraProps> *cameras)
{
	cameras_ = cameras;

	int ret = yaml_parser_initialize(&parser_);
	if (!ret) {
		LOG(HALConfig, Fatal) << "Failed to initialize yaml parser";
		return -EINVAL;
	}
	yaml_parser_set_input_file(&parser_, fh);

	yaml_token_t token;
	yaml_parser_scan(&parser_, &token);
	if (token.type != YAML_STREAM_START_TOKEN) {
		LOG(HALConfig, Error) << "Configuration file is not valid";
		yaml_token_delete(&token);
		yaml_parser_delete(&parser_);
		return -EINVAL;
	}
	yaml_token_delete(&token);

	yaml_parser_scan(&parser_, &token);
	if (token.type != YAML_BLOCK_MAPPING_START_TOKEN) {
		LOG(HALConfig, Error) << "Configuration file is not valid";
		yaml_token_delete(&token);
		yaml_parser_delete(&parser_);
		return -EINVAL;
	}
	yaml_token_delete(&token);

	/* Parse the file and parse each single key one by one. */
	do {
		yaml_parser_scan(&parser_, &token);
		switch (token.type) {
		case YAML_KEY_TOKEN:
			yaml_token_delete(&token);
			ret = parseEntry();
			break;

		case YAML_STREAM_END_TOKEN:
			ret = -ENOENT;
			[[fallthrough]];
		default:
			yaml_token_delete(&token);
			break;
		}
	} while (ret >= 0);
	yaml_parser_delete(&parser_);

	if (ret && ret != -ENOENT)
		LOG(HALConfig, Error) << "Configuration file is not valid";

	return ret == -ENOENT ? 0 : ret;
}

std::filesystem::path CameraHalConfig::findFilePath(const std::string &filename) const
{
	std::filesystem::path filePath = filename;
	if (std::filesystem::is_regular_file(filePath))
		return filename;

	std::filesystem::path root = utils::libcameraSourcePath();
	if (!root.empty()) {
		filePath = root / "data" / filename;
		if (std::filesystem::is_regular_file(filePath))
			return filePath;
	}

	root = LIBCAMERA_SYSCONF_DIR;
	filePath = root / filename;
	if (std::filesystem::is_regular_file(filePath))
		return filePath;

	return "";
}

FILE *CameraHalConfig::openConfigFile(const std::string &filename)
{
	const std::filesystem::path filePath = findFilePath(filename);
	if (filePath.empty()) {
		LOG(HALConfig, Error)
			<< "Configuration file: \"" << filename << "\" not found";
		return nullptr;
	}

	LOG(HALConfig, Debug) << "Reading configuration file from " << filePath;

	FILE *fh = fopen(filePath.c_str(), "r");
	if (!fh) {
		int ret = -errno;
		LOG(HALConfig, Error) << "Failed to open configuration file "
				      << filePath << ": " << strerror(-ret);
		return nullptr;
	}

	return fh;
}

CameraHalConfig::CameraHalConfig()
	: Extensible(new Private(this)), exists_(false), valid_(false)
{
}

/*
 * Open the HAL configuration file and validate its content.
 * Return 0 on success, a negative error code otherwise
 * retval -ENOENT The configuration file is not available
 * retval -EINVAL The configuration file is available but not valid
 */
int CameraHalConfig::open()
{
	FILE *fh = openConfigFile("camera_hal.yaml");
	if (!fh)
		return -ENOENT;

	exists_ = true;

	Private *const d = LIBCAMERA_D_PTR();
	int ret = d->parseConfigFile(fh, &cameras_);
	fclose(fh);
	if (ret)
		return -EINVAL;

	valid_ = true;

	for (const auto &c : cameras_) {
		const std::string &cameraId = c.first;
		const CameraProps &camera = c.second;
		LOG(HALConfig, Debug) << "'" << cameraId << "' "
				      << "(" << camera.facing << ")["
				      << camera.rotation << "]";
	}

	return 0;
}

const CameraProps &CameraHalConfig::cameraProps(const std::string &cameraId) const
{
	static CameraProps empty;

	const auto &it = cameras_.find(cameraId);
	if (it == cameras_.end()) {
		LOG(HALConfig, Error)
			<< "Camera '" << cameraId
			<< "' not described in the HAL configuration file";
		return empty;
	}

	return it->second;
}
