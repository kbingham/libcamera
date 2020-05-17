/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020, Ideas on Board Oy
 *
 * kms_sink.h - KMS Sink
 */
#ifndef __CAM_KMS_SINK_H__
#define __CAM_KMS_SINK_H__

#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include <libcamera/geometry.h>
#include <libcamera/pixelformats.h>
#include <libcamera/signal.h>

#include "drm.h"
#include "frame_sink.h"

class KMSSink : public FrameSink
{
public:
	KMSSink(const std::string &connectorName);

	bool isValid() const { return connector_ != nullptr; }

	void mapBuffer(libcamera::FrameBuffer *buffer) override;

	int configure(const libcamera::CameraConfiguration &config) override;
	int start() override;
	int stop() override;

	bool consumeBuffer(const libcamera::Stream *stream,
			   libcamera::FrameBuffer *buffer) override;

private:
	class Request
	{
	public:
		Request(DRM::AtomicRequest *request, libcamera::FrameBuffer *buffer)
			: request_(request), buffer_(buffer)
		{
		}

		std::unique_ptr<DRM::AtomicRequest> request_;
		libcamera::FrameBuffer *buffer_;
	};

	int configurePipeline(const libcamera::PixelFormat &format);
	void requestComplete(DRM::AtomicRequest *request);

	DRM::Device dev_;

	const DRM::Connector *connector_;
	const DRM::Crtc *crtc_;
	const DRM::Plane *plane_;
	const DRM::Mode *mode_;

	libcamera::PixelFormat format_;
	libcamera::Size size_;
	unsigned int stride_;

	bool planeInitialized_;

	std::map<libcamera::FrameBuffer *, std::unique_ptr<DRM::FrameBuffer>> buffers_;

	std::mutex lock_;
	std::unique_ptr<Request> pending_;
	std::unique_ptr<Request> queued_;
	std::unique_ptr<Request> active_;
};

#endif /* __CAM_KMS_SINK_H__ */
