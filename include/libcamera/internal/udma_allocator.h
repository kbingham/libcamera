/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Ideas on Board Oy
 *
 * Allocate buffers for use and sharing from /dev/udmabuf
 */

#pragma once

#include <stddef.h>
#include <vector>

#include <libcamera/base/unique_fd.h>

namespace libcamera {

class Camera;
class Stream;
class FrameBuffer;

class UdmaBuf
{
public:
	UdmaBuf();
	bool isValid() const { return allocator_.isValid(); }
	UniqueFD allocate(const char *name, std::size_t size);

	int exportFrameBuffers(Camera *camera, Stream *stream,
			       std::vector<std::unique_ptr<FrameBuffer>> *buffers);

private:
	UniqueFD allocator_;
	std::unique_ptr<FrameBuffer> createBuffer(std::size_t size);
};

} /* namespace libcamera */
