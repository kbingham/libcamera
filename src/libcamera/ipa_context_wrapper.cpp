/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * ipa_context_wrapper.cpp - Image Processing Algorithm context wrapper
 */

#include "ipa_context_wrapper.h"

#include <libcamera/controls.h>

/**
 * \file ipa_context_wrapper.h
 * \brief Image Processing Algorithm context wrapper
 */

namespace libcamera {

/**
 * \class IPAContextWrapper
 * \brief Wrap an ipa_context and expose it as an IPAInterface
 *
 * The IPAContextWrapper class wraps an ipa_context, provided by an IPA module, and
 * exposes an IPAInterface. This mechanism is used for IPAs that are not
 * isolated in a separate process to allow direct calls from pipeline handler
 * using the IPAInterface API instead of the lower-level ipa_context API.
 *
 * The IPAInterface methods are converted to the ipa_context API by translating
 * all C++ arguments into plain C structures or byte arrays that contain no
 * pointer, as required by the ipa_context API.
 */

/**
 * \brief Construct an IPAContextWrapper instance that wraps the \a context
 * \param[in] context The IPA module context
 *
 * Ownership of the \a context is passed to the IPAContextWrapper. The context remains
 * valid for the whole lifetime of the wrapper and is destroyed automatically
 * with it.
 */
IPAContextWrapper::IPAContextWrapper(struct ipa_context *context)
	: ctx_(context)
{
	if (ctx_ && ctx_->ops->get_interface) {
		intf_ = reinterpret_cast<IPAInterface *>(ctx_->ops->get_interface(ctx_));
		intf_->queueFrameAction.connect(this, &IPAContextWrapper::queueFrameAction);
	} else {
		intf_ = nullptr;
	}
}

IPAContextWrapper::~IPAContextWrapper()
{
	if (ctx_)
		ctx_->ops->destroy(ctx_);
}

int IPAContextWrapper::init()
{
	if (intf_)
		return intf_->init();

	if (!ctx_)
		return 0;

	ctx_->ops->register_callbacks(ctx_, &IPAContextWrapper::callbacks_, this);
	ctx_->ops->init(ctx_);

	return 0;
}

void IPAContextWrapper::configure(const std::map<unsigned int, IPAStream> &streamConfig,
				  const std::map<unsigned int, ControlInfoMap> &entityControls)
{
	if (intf_)
		return intf_->configure(streamConfig, entityControls);

	if (!ctx_)
		return;

	ctx_->ops->configure(ctx_);
}

void IPAContextWrapper::mapBuffers(const std::vector<IPABuffer> &buffers)
{
	if (intf_)
		return intf_->mapBuffers(buffers);

	if (!ctx_)
		return;

	struct ipa_buffer c_buffers[buffers.size()];

	for (unsigned int i = 0; i < buffers.size(); ++i) {
		struct ipa_buffer &c_buffer = c_buffers[i];
		const IPABuffer &buffer = buffers[i];
		const std::vector<Plane> &planes = buffer.memory.planes();

		c_buffer.id = buffer.id;
		c_buffer.num_planes = planes.size();

		for (unsigned int j = 0; j < planes.size(); ++j) {
			const Plane &plane = planes[j];
			c_buffer.planes[j].dmabuf = plane.dmabuf();
			c_buffer.planes[j].length = plane.length();
		}
	}

	ctx_->ops->map_buffers(ctx_, c_buffers, buffers.size());
}

void IPAContextWrapper::unmapBuffers(const std::vector<unsigned int> &ids)
{
	if (intf_)
		return intf_->unmapBuffers(ids);

	if (!ctx_)
		return;

	ctx_->ops->unmap_buffers(ctx_, ids.data(), ids.size());
}

void IPAContextWrapper::processEvent(const IPAOperationData &data)
{
	if (intf_)
		return intf_->processEvent(data);

	if (!ctx_)
		return;

	ctx_->ops->process_event(ctx_);
}

void IPAContextWrapper::queueFrameAction(unsigned int frame,
					 const IPAOperationData &data)
{
	IPAInterface::queueFrameAction.emit(frame, data);
}

void IPAContextWrapper::queue_frame_action(void *ctx, unsigned int frame)
{
	IPAContextWrapper *_this = static_cast<IPAContextWrapper *>(ctx);
	IPAOperationData data;

	_this->queueFrameAction(frame, data);
}

#ifndef __DOXYGEN__
/*
 * This construct confuses Doygen and makes it believe that all members of the
 * operations is a member of IPAInterfaceWrapper. It must thus be hidden.
 */
const struct ipa_callback_ops IPAContextWrapper::callbacks_ = {
	.queue_frame_action = &IPAContextWrapper::queue_frame_action,
};
#endif

} /* namespace libcamera */
