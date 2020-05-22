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

	/* Use a format supported by the viewfinder if available. */
	std::vector<PixelFormat> formats = cfg.formats().pixelformats();
	std::cout << "StreamConfiguration supports " << formats.size() << " formats:" << std::endl;
	for (const PixelFormat &format : formats)
	{
		std::cout << " - " << format.toString() << std::endl;
	}

	switch (cfg.pixelFormat.fourcc()) {
	case DRM_FORMAT_YUYV:
		std::cerr << "Input format is YUYV" << std::endl;
		subSampling_ = TJSAMP_422;
		isYUV_ = true;
		break;

	case DRM_FORMAT_BGRA8888:
		std::cerr << "Input format is BGRA8888" << std::endl;
		pixelFormat_ = TJPF_RGBX;
		subSampling_ = TJSAMP_444;
		isYUV_ = false;
		break;
	case DRM_FORMAT_ARGB8888:
		std::cerr << "Input format is ARGB8888" << std::endl;
		pixelFormat_ = TJPF_RGBX;
		subSampling_ = TJSAMP_444;
		isYUV_ = false;
		break;

	case DRM_FORMAT_RGB888:
		std::cerr << "Input format is RGB888" << std::endl;
		pixelFormat_ = TJPF_RGB;
		subSampling_ = TJSAMP_444;
		isYUV_ = false;
		break;
	case DRM_FORMAT_BGR888:
		std::cerr << "Input format is BGR888" << std::endl;
		pixelFormat_ = TJPF_BGR;
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
		std::cout << "Width: " << width_ << " height: " << height_ << " stride: " << stride_ << std::endl;
		std::cout << "width * tjPixelSize[pixelformat] = " << width_ * tjPixelSize[pixelFormat_] << std::endl;

		ret = tjCompress2(compressor_, frame->memory[0].data,
				  width_, 0 /*stride_*/, height_,
				  pixelFormat_, &data, &length, jpegSubSampling_, quality_, flags);

		jpeg->data = data;
		jpeg->length = length;

		return ret;
	}


	/* int tjCompressFromYUV(tjhandle handle, const unsigned char *srcBuf, int width,
		int pad, int height, int subsamp, unsigned char **jpegBuf,
		unsigned long *jpegSize, int jpegQual, int flags) */

	int pad = stride_; // Validate this is the right parameter
	ret = tjCompressFromYUV(compressor_, frame->memory[0].data,
				width_, pad, height_, subSampling_,
				&data, &length, quality_, flags);


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
