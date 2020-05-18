/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * capture.cpp - Cam capture
 */

#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits.h>
#include <sstream>

#include "buffer_writer.h"
#include "capture.h"
#include "main.h"

using namespace libcamera;

Capture::Capture(std::shared_ptr<Camera> camera, CameraConfiguration *config,
		 const StreamRoles &roles)
	: camera_(camera), config_(config), roles_(roles), sink_(nullptr)
{
}

int Capture::run(EventLoop *loop, const OptionsParser::Options &options)
{
	int ret;

	if (!camera_) {
		std::cout << "Can't capture without a camera" << std::endl;
		return -ENODEV;
	}

	ret = camera_->configure(config_);
	if (ret < 0) {
		std::cout << "Failed to configure camera" << std::endl;
		return ret;
	}

	streamName_.clear();
	for (unsigned int index = 0; index < config_->size(); ++index) {
		StreamConfiguration &cfg = config_->at(index);
		streamName_[cfg.stream()] = "stream" + std::to_string(index);
	}

	camera_->requestCompleted.connect(this, &Capture::requestComplete);

	if (options.isSet(OptFile)) {
		if (!options[OptFile].toString().empty())
			sink_ = new BufferWriter(options[OptFile]);
		else
			sink_ = new BufferWriter();
	}

	if (sink_) {
		ret = sink_->configure(*config_);
		if (ret < 0) {
			std::cout << "Failed to configure frame sink"
				  << std::endl;
			return ret;
		}

		sink_->bufferReleased.connect(this, &Capture::sinkRelease);
	}

	FrameBufferAllocator *allocator = new FrameBufferAllocator(camera_);

	ret = capture(loop, allocator);

	delete sink_;
	sink_ = nullptr;

	delete allocator;

	return ret;
}

int Capture::capture(EventLoop *loop, FrameBufferAllocator *allocator)
{
	int ret;

	/* Identify the stream with the least number of buffers. */
	unsigned int nbuffers = UINT_MAX;
	for (StreamConfiguration &cfg : *config_) {
		ret = allocator->allocate(cfg.stream());
		if (ret < 0) {
			std::cerr << "Can't allocate buffers" << std::endl;
			return -ENOMEM;
		}

		unsigned int allocated = allocator->buffers(cfg.stream()).size();
		nbuffers = std::min(nbuffers, allocated);
	}

	/*
	 * TODO: make cam tool smarter to support still capture by for
	 * example pushing a button. For now run all streams all the time.
	 */

	std::vector<Request *> requests;
	for (unsigned int i = 0; i < nbuffers; i++) {
		Request *request = camera_->createRequest();
		if (!request) {
			std::cerr << "Can't create request" << std::endl;
			return -ENOMEM;
		}

		for (StreamConfiguration &cfg : *config_) {
			Stream *stream = cfg.stream();
			const std::vector<std::unique_ptr<FrameBuffer>> &buffers =
				allocator->buffers(stream);
			const std::unique_ptr<FrameBuffer> &buffer = buffers[i];

			ret = request->addBuffer(stream, buffer.get());
			if (ret < 0) {
				std::cerr << "Can't set buffer for request"
					  << std::endl;
				return ret;
			}

			if (sink_)
				sink_->mapBuffer(buffer.get());
		}

		requests.push_back(request);
	}

	if (sink_) {
		ret = sink_->start();
		if (ret) {
			std::cout << "Failed to start frame sink" << std::endl;
			return ret;
		}
	}

	ret = camera_->start();
	if (ret) {
		std::cout << "Failed to start capture" << std::endl;
		if (sink_)
			sink_->stop();
		return ret;
	}

	for (Request *request : requests) {
		ret = camera_->queueRequest(request);
		if (ret < 0) {
			std::cerr << "Can't queue request" << std::endl;
			camera_->stop();
			if (sink_)
				sink_->stop();
			return ret;
		}
	}

	std::cout << "Capture until user interrupts by SIGINT" << std::endl;
	ret = loop->exec();
	if (ret)
		std::cout << "Failed to run capture loop" << std::endl;

	ret = camera_->stop();
	if (ret)
		std::cout << "Failed to stop capture" << std::endl;

	if (sink_) {
		ret = sink_->stop();
		if (ret)
			std::cout << "Failed to stop frame sink" << std::endl;
	}

	return ret;
}

void Capture::requestComplete(Request *request)
{
	if (request->status() == Request::RequestCancelled)
		return;

	const std::map<Stream *, FrameBuffer *> &buffers = request->buffers();

	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	double fps = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_).count();
	fps = last_ != std::chrono::steady_clock::time_point() && fps
	    ? 1000.0 / fps : 0.0;
	last_ = now;

	bool requeue = true;

	std::stringstream info;
	info << "fps: " << std::fixed << std::setprecision(2) << fps;

	for (auto it = buffers.begin(); it != buffers.end(); ++it) {
		Stream *stream = it->first;
		FrameBuffer *buffer = it->second;

		const FrameMetadata &metadata = buffer->metadata();

		info << " " << streamName_[stream]
		     << " seq: " << std::setw(6) << std::setfill('0') << metadata.sequence
		     << " bytesused: ";

		unsigned int nplane = 0;
		for (const FrameMetadata::Plane &plane : metadata.planes) {
			info << plane.bytesused;
			if (++nplane < metadata.planes.size())
				info << "/";
		}

		if (sink_) {
			if (!sink_->consumeBuffer(stream, buffer))
				requeue = false;
		}
	}

	std::cout << info.str() << std::endl;

	/*
	 * If the frame sink holds on the buffer, we'll requeue it later in the
	 * complete handler.
	 */
	if (!requeue)
		return;

	/*
	 * Create a new request and populate it with one buffer for each
	 * stream.
	 */
	request = camera_->createRequest();
	if (!request) {
		std::cerr << "Can't create request" << std::endl;
		return;
	}

	for (auto it = buffers.begin(); it != buffers.end(); ++it) {
		Stream *stream = it->first;
		FrameBuffer *buffer = it->second;

		request->addBuffer(stream, buffer);
	}

	camera_->queueRequest(request);
}

void Capture::sinkRelease(libcamera::FrameBuffer *buffer)
{
	Request *request = camera_->createRequest();
	if (!request) {
		std::cerr << "Can't create request" << std::endl;
		return;
	}

	request->addBuffer(config_->at(0).stream(), buffer);

	camera_->queueRequest(request);
}
