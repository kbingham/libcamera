/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * file_sink.h - File Sink
 */
#ifndef __TURBO_JPEG_COMPRESSOR_H__
#define __TURBO_JPEG_COMPRESSOR_H__

#include <libcamera/buffer.h>
#include <libcamera/stream.h>

#include <turbojpeg.h>

struct Frame {
	libcamera::FrameBuffer *buffer;
	libcamera::FrameMetadata *metadata;
	struct {
		const unsigned char *data;
		unsigned int length;
	} memory[3];
};

struct TJJPEGImage {
	unsigned char *data;
	unsigned int length;
};

class TurboJPEGCompressor
{
public:
	TurboJPEGCompressor();
	~TurboJPEGCompressor();

	int configure(const libcamera::StreamConfiguration &cfg);
	int compress(Frame *frame, TJJPEGImage *jpeg);

private:
	tjhandle compressor_;
	unsigned int quality_;
	int pixelFormat_;
	bool isYUV_;
	int subSampling_;
	int jpegSubSampling_;
	unsigned int maxBufferSize_;

	unsigned int width_;
	unsigned int height_;
	unsigned int stride_;
};

#endif /* __CAM_FILE_SINK_JPEG_H__ */
