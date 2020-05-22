/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * turbojpeg_compressor.cpp - JPEG compression using libjpeg-turbo
 */

#include "turbojpeg_compressor.h"

#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <libcamera/camera.h>

using namespace libcamera;

TurboJPEGCompressor::TurboJPEGCompressor()
	: compressor_(nullptr), quality_(95), pixelFormat_(TJPF_UNKNOWN),
	  isYUV_(false), subSampling_(TJSAMP_444), jpegSubSampling_(TJSAMP_422)
{
	compressor_ = tjInitCompress();
	if (!compressor_)
		std::cerr << "Failed to create compressor." << std::endl;
}

TurboJPEGCompressor::~TurboJPEGCompressor()
{
	if (compressor_)
		tjDestroy(compressor_);
}

int TurboJPEGCompressor::configure(const StreamConfiguration &cfg)
{
	std::cout << "Configuring pixelformat as : "
		  << cfg.pixelFormat.toString() << std::endl;

	width_ = cfg.size.width;
	stride_ = cfg.stride;
	height_ = cfg.size.height;

	switch (cfg.pixelFormat.fourcc()) {
	// TJSAMP_GRAY, TJSAMP_444, TJSAMP_422, TJSAMP_420
	case DRM_FORMAT_YUYV:
		std::cerr << "Input format is YUYV" << std::endl;
		subSampling_ = TJSAMP_420;
		isYUV_ = true;
		break;
	case DRM_FORMAT_ARGB8888:
		pixelFormat_ = TJPF_ARGB;
		subSampling_ = TJSAMP_444;
		isYUV_ = false;
		break;
	case DRM_FORMAT_MJPEG:
		std::cerr << "Input format is already MJPEG" << std::endl;
		/* Fallthrough */
	default:
		std::cerr << "Unhandled pixelformat "
			  << cfg.pixelFormat.toString() << std::endl;
		return -1;
	}

	/*
	 * Note (from turbojpeg.h): The number of bytes returned by this function is
	 * larger than the size of the uncompressed source image. The reason for this
	 * is that the JPEG format uses 16-bit coefficients, and it is thus possible
	 * for a very high-quality JPEG image with very high-frequency content to
	 * expand rather than compress when converted to the JPEG format. Such images
	 * represent a very rare corner case, but since there is no way to predict the
	 * size of a JPEG image prior to compression, the corner case has to be
	 * handled.
	 */
	maxBufferSize_ = tjBufSize(width_, height_, jpegSubSampling_);

	return 0;
}

int TurboJPEGCompressor::compress(Frame *frame, TJJPEGImage *jpeg)
{
	unsigned char *data = NULL;
	unsigned long length = 0;
	unsigned int flags = 0;
	int ret = -1;

	/* RGB compression */
	if (!isYUV_) {
		ret = tjCompress2(compressor_, frame->memory[0].data,
				  width_, stride_, height_,
				  pixelFormat_, &data, &length, jpegSubSampling_, quality_, flags);
	} else {
		/* int tjCompressFromYUV(tjhandle handle, const unsigned char *srcBuf, int width,
			 int pad, int height, int subsamp, unsigned char **jpegBuf,
			 unsigned long *jpegSize, int jpegQual, int flags) */
		int pad = stride_; // Validate this is the right parameter
		ret = tjCompressFromYUV(compressor_, frame->memory[0].data,
					width_, pad, height_, subSampling_,
					&data, &length, quality_, flags);
	}

	if (ret != 0) {
		std::cerr << "TurbJPEG failed to compress frame: ("
			  << tjGetErrorCode(compressor_)
			  << ") " << tjGetErrorStr2(compressor_)
			  << std::endl;
	}

	jpeg->data = data;
	jpeg->length = length;

	return ret;
}
