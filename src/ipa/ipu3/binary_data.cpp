/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * binary_data.cpp - AIQ Binary Data Wrapper
 */

#include "binary_data.h"

#include "libcamera/internal/file.h"
#include "libcamera/internal/log.h"

namespace libcamera::ipa::ipu3 {

LOG_DEFINE_CATEGORY(AIBD)

/**
 * \class AIQBinaryData
 * \brief Binary Data wrapper
 *
 * Loads data from a file, and returns it as an ia_binary_data type.
 * Data is freed automatically when the object goes out of scope.
 */

BinaryData::BinaryData()
{
	iaBinaryData_.data = nullptr;
	iaBinaryData_.size = 0;
}

int BinaryData::load(const char *filename)
{
	File binary(filename);

	if (!binary.exists()) {
		LOG(AIBD, Error) << "Failed to find file: " << filename;
		return -ENOENT;
	}

	if (!binary.open(File::ReadOnly)) {
		LOG(AIBD, Error) << "Failed to open: " << filename;
		return -EINVAL;
	}

	ssize_t fileSize = binary.size();
	if (fileSize < 0) {
		LOG(AIBD, Error) << "Failed to determine fileSize: " << filename;
		return -ENODATA;
	}

	data_.resize(fileSize);

	int bytesRead = binary.read(data_);
	if (bytesRead != fileSize) {
		LOG(AIBD, Error) << "Failed to read file: " << filename;
		return -EINVAL;
	}

	iaBinaryData_.data = data_.data();
	iaBinaryData_.size = fileSize;

	LOG(AIBD, Info) << "Successfully loaded: " << filename;

	return 0;
}

} /* namespace libcamera::ipa::ipu3 */
