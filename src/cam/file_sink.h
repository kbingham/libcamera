/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * file_sink.h - File Sink
 */
#ifndef __CAM_FILE_SINK_H__
#define __CAM_FILE_SINK_H__

#include <map>
#include <string>

#include <libcamera/buffer.h>

#include "frame_sink.h"

class FileSink : public FrameSink
{
public:
	FileSink(const std::string &pattern = "frame-#.bin");
	~FileSink();

	int configure(const libcamera::CameraConfiguration &config) override;

	void mapBuffer(libcamera::FrameBuffer *buffer) override;

	bool consumeBuffer(const libcamera::Stream *stream,
			   libcamera::FrameBuffer *buffer) override;

private:
	std::map<const libcamera::Stream *, std::string> streamNames_;
	std::string pattern_;
	std::map<int, std::pair<void *, unsigned int>> mappedBuffers_;
};

#endif /* __CAM_FILE_SINK_H__ */
