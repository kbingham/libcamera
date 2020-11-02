/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * frames.cpp - Intel IPU3 Frames helper
 */

#include "frames.h"

#include <libcamera/buffer.h>
#include <libcamera/request.h>

#include <libcamera/ipa/ipa_interface.h>

#include "libcamera/internal/pipeline_handler.h"
#include "libcamera/internal/v4l2_videodevice.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(IPU3)

IPU3Frames::IPU3Frames(PipelineHandler *pipe, IPAProxy *ipa)
	: pipe_(pipe), ipa_(ipa), nextId_(0)
{
}

void IPU3Frames::mapBuffers(const std::vector<std::unique_ptr<FrameBuffer>> &paramBuffers,
			    const std::vector<std::unique_ptr<FrameBuffer>> &statBuffers)
{
	unsigned int ipaBufferId = 1;

	for (const std::unique_ptr<FrameBuffer> &buffer : paramBuffers) {
		buffer->setCookie(ipaBufferId++);
		ipaBuffers_.push_back({ .id = buffer->cookie(),
					.planes = buffer->planes() });
		availableParamBuffers_.push(buffer.get());
	}

	for (const std::unique_ptr<FrameBuffer> &buffer : statBuffers) {
		buffer->setCookie(ipaBufferId++);
		ipaBuffers_.push_back({ .id = buffer->cookie(),
					.planes = buffer->planes() });
		availableStatBuffers_.push(buffer.get());
	}

	ipa_->mapBuffers(ipaBuffers_);

	nextId_ = 0;
	frameInfo_.clear();
}

void IPU3Frames::unmapBuffers()
{
	availableParamBuffers_ = {};
	availableStatBuffers_ = {};

	std::vector<unsigned int> ids;
	for (IPABuffer &ipabuf : ipaBuffers_)
		ids.push_back(ipabuf.id);

	ipa_->unmapBuffers(ids);
	ipaBuffers_.clear();
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

	pipe_->completeRequest(request);

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
