/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * capture.h - Cam capture
 */
#ifndef __CAM_CAPTURE_H__
#define __CAM_CAPTURE_H__

#include <chrono>
#include <memory>

#include <libcamera/buffer.h>
#include <libcamera/camera.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>

#include "event_loop.h"
#include "options.h"

class FrameSink;

class Capture
{
public:
	Capture(std::shared_ptr<libcamera::Camera> camera,
		libcamera::CameraConfiguration *config,
		const libcamera::StreamRoles &roles);

	int run(EventLoop *loop, const OptionsParser::Options &options);
private:
	int capture(EventLoop *loop,
		    libcamera::FrameBufferAllocator *allocator);

	void requestComplete(libcamera::Request *request);
	void sinkRelease(libcamera::FrameBuffer *buffer);

	std::shared_ptr<libcamera::Camera> camera_;
	libcamera::CameraConfiguration *config_;
	libcamera::StreamRoles roles_;

	std::map<libcamera::Stream *, std::string> streamName_;
	FrameSink *sink_;
	std::chrono::steady_clock::time_point last_;
};

#endif /* __CAM_CAPTURE_H__ */
