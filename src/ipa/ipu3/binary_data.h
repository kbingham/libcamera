/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * binary_data.h - AIQ Binary Data Wrapper
 */

#include <vector>

#include <ia_imaging/ia_aiq.h>

#include <libcamera/class.h>

#ifndef __IPA_IPU3_BINARY_DATA__
#define __IPA_IPU3_BINARY_DATA__

namespace libcamera::ipa::ipu3 {

class BinaryData
{
public:
	BinaryData();

	int load(const char *filename);
	ia_binary_data *data() { return &iaBinaryData_; }

private:
	LIBCAMERA_DISABLE_COPY_AND_MOVE(BinaryData)

	ia_binary_data iaBinaryData_;
	std::vector<uint8_t> data_;
};

} /* namespace libcamera::ipa::ipu3 */

#endif /* __IPA_IPU3_BINARY_DATA__ */

