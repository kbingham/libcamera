/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Google Inc.
 *
 * fc_queue.h - IPA Frame context queue
 *
 */

#pragma once

#include <array>

#include <libcamera/base/log.h>

#include <libcamera/controls.h>
#include <libcamera/request.h>

namespace libcamera {

LOG_DECLARE_CATEGORY(FCQueue)

namespace ipa {

/*
 * Maximum number of frame contexts to be held onto
 *
 * \todo Should be larger than ISP delay + sensor delay
 */
static constexpr uint32_t kMaxFrameContexts = 16;

struct IPAFrameContext {
	unsigned int frame;
	Request::ErrorFlags error;
};

template<typename FrameContext>
class FCQueue : public std::array<FrameContext, kMaxFrameContexts>
{
private:
	void initialise(FrameContext &frameContext, const uint32_t frame)
	{
		frameContext = {};
		frameContext.frame = frame;
	}

public:
	void clear()
	{
		this->fill({});
	}

	FrameContext &initialise(const uint32_t frame)
	{
		FrameContext &frameContext = this->at(frame & kMaxFrameContexts);

		/*
		 * Do not re-initialise if a get() call has already fetched this
		 * frame context to preseve the error flags already raised.
		 */
		if (frame == frameContext.frame && frame != 0) {
			LOG(FCQueue, Warning)
				<< "Frame " << frame << " already initialised";
		} else {
			initialise(frameContext, frame);
		}

		return frameContext;
	}

	FrameContext &get(uint32_t frame)
	{
		FrameContext &frameContext = this->at(frame & kMaxFrameContexts);

		if (frame != frameContext.frame) {
			LOG(FCQueue, Warning)
				<< "Obtained an uninitialised FrameContext for "
				<< frame;

			initialise(frameContext, frame);

			/*
			 * The frame context has been retrieved before it was
			 * initialised through the initialise() call. This
			 * indicates an algorithm attempted to access a Frame
			 * context before it was queued to the IPA.
			 *
			 * Controls applied for this request may be left
			 * unhandled.
			 */
			frameContext.error |= Request::PFCError;
		}

		return frameContext;
	}
};

} /* namespace ipa */

} /* namespace libcamera */
