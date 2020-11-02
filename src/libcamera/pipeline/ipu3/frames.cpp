/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * frames.cpp - Intel IPU3 Frames helper
 */

#include "frames.h"

#include <libcamera/buffer.h>
#include <libcamera/request.h>

#include "libcamera/internal/pipeline_handler.h"
#include "libcamera/internal/v4l2_videodevice.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(IPU3)

IPU3Frames::IPU3Frames()
	: nextId_(0)
{
}

void IPU3Frames::init(const std::vector<std::unique_ptr<FrameBuffer>> &paramBuffers,
		      const std::vector<std::unique_ptr<FrameBuffer>> &statBuffers)
{
	for (const std::unique_ptr<FrameBuffer> &buffer : paramBuffers)
		availableParamBuffers_.push(buffer.get());

	for (const std::unique_ptr<FrameBuffer> &buffer : statBuffers)
		availableStatBuffers_.push(buffer.get());

	nextId_ = 0;
	frameInfo_.clear();
}

void IPU3Frames::clear()
{
	availableParamBuffers_ = {};
	availableStatBuffers_ = {};
}

IPU3Frames::Info *IPU3Frames::create(Request *request)
{
	unsigned int id = nextId_++;

	if (availableParamBuffers_.empty()) {
		LOG(IPU3, Error) << "Parameters buffer underrun";
		return nullptr;
	}
	FrameBuffer *paramBuffer = availableParamBuffers_.front();

	if (availableStatBuffers_.empty()) {
		LOG(IPU3, Error) << "Statisitc buffer underrun";
		return nullptr;
	}
	FrameBuffer *statBuffer = availableStatBuffers_.front();

	availableParamBuffers_.pop();
	availableStatBuffers_.pop();

	std::unique_ptr<Info> info = std::make_unique<Info>();

	info->id = id;
	info->request = request;
	info->rawBuffer = nullptr;
	info->paramBuffer = paramBuffer;
	info->statBuffer = statBuffer;
	info->paramFilled = false;
	info->paramDequeued = false;
	info->metadataProcessed = false;

	frameInfo_[id] = std::move(info);

	return frameInfo_[id].get();
}

bool IPU3Frames::tryComplete(IPU3Frames::Info *info)
{
	Request *request = info->request;

	if (request->hasPendingBuffers())
		return false;

	if (!info->metadataProcessed)
		return false;

	if (!info->paramDequeued)
		return false;

	/* Return params and stat buffer for reuse. */
	availableParamBuffers_.push(info->paramBuffer);
	availableStatBuffers_.push(info->statBuffer);

	/* Delete the extended frame information. */
	frameInfo_.erase(info->id);

	return true;
}

IPU3Frames::Info *IPU3Frames::find(unsigned int id)
{
	const auto &itInfo = frameInfo_.find(id);

	if (itInfo != frameInfo_.end())
		return itInfo->second.get();

	return nullptr;
}

IPU3Frames::Info *IPU3Frames::find(FrameBuffer *buffer)
{
	for (auto const &itInfo : frameInfo_) {
		Info *info = itInfo.second.get();

		for (auto const itBuffers : info->request->buffers())
			if (itBuffers.second == buffer)
				return info;

		if (info->rawBuffer == buffer || info->paramBuffer == buffer ||
		    info->statBuffer == buffer)
			return info;
	}

	return nullptr;
}

IPU3Frames::Info *IPU3Frames::find(Request *request)
{
	for (auto const &itInfo : frameInfo_) {
		Info *info = itInfo.second.get();

		if (info->request == request)
			return info;
	}

	return nullptr;
}

} /* namespace libcamera */
