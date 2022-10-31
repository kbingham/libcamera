/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Ideas on Board Oy
 *
 * Allocate buffers for use and sharing from /dev/udmabuf
 */

#include "libcamera/internal/udma_allocator.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/udmabuf.h>

#include <libcamera/base/log.h>

#include <libcamera/stream.h>

namespace libcamera {

LOG_DEFINE_CATEGORY(UDMA)

UdmaBuf::UdmaBuf()
{
	int ret = ::open("/dev/udmabuf", O_RDWR, 0);
	if (ret < 0) {
		ret = errno;
		LOG(UDMA, Error)
			<< "Failed to open allocator: " << strerror(ret);

		if (ret == EACCES) {
			LOG(UDMA, Info)
				<< "Consider making /dev/udmabuf accessible by the video group";
			LOG(UDMA, Info)
				<< "Alternatively, add your user to the kvm group.";
		}
	} else {
		allocator_ = UniqueFD(ret);
	}
}

UniqueFD UdmaBuf::allocate(const char *name, std::size_t size)
{
	if (!isValid()) {
		LOG(UDMA, Fatal) << "Allocation attempted without allocator" << name;
		return {};
	}

	int ret;

	ret = memfd_create(name, MFD_ALLOW_SEALING);
	if (ret < 0) {
		ret = errno;
		LOG(UDMA, Error)
			<< "Failed to allocate memfd storage: "
			<< strerror(ret);
		return {};
	}

	UniqueFD memfd(ret);

	ret = ftruncate(memfd.get(), size);
	if (ret < 0) {
		ret = errno;
		LOG(UDMA, Error)
			<< "Failed to set memfd size: " << strerror(ret);
		return {};
	}

	/* UDMA Buffers *must* have the F_SEAL_SHRINK seal */
	ret = fcntl(memfd.get(), F_ADD_SEALS, F_SEAL_SHRINK);
	if (ret < 0) {
		ret = errno;
		LOG(UDMA, Error)
			<< "Failed to seal the memfd: " << strerror(ret);
		return {};
	}

	struct udmabuf_create create;

	create.memfd = memfd.get();
	create.flags = UDMABUF_FLAGS_CLOEXEC;
	create.offset = 0;
	create.size = size;

	ret = ::ioctl(allocator_.get(), UDMABUF_CREATE, &create);
	if (ret < 0) {
		ret = errno;
		LOG(UDMA, Error)
			<< "Failed to allocate " << size << " bytes: "
			<< strerror(ret);
		return {};
	}

	/* The underlying memfd is kept as as a reference in the kernel */
	UniqueFD uDma(ret);

	if (create.size != size)
		LOG(UDMA, Warning)
			<< "Allocated " << create.size << " bytes instead of "
			<< size << " bytes";

	/* Fail if not suitable, the allocation will be free'd by UniqueFD */
	if (create.size < size)
		return {};

	LOG(UDMA, Debug) << "Allocated " << create.size << " bytes";

	return uDma;
}

std::unique_ptr<FrameBuffer> UdmaBuf::createBuffer(std::size_t size)
{
	std::vector<FrameBuffer::Plane> planes;

	UniqueFD fd = allocate("Buffer", size);
	if (!fd.isValid())
		return nullptr;

	FrameBuffer::Plane plane;
	plane.fd = SharedFD(std::move(fd));
	plane.offset = 0;
	plane.length = size;

	planes.push_back(std::move(plane));

	return std::make_unique<FrameBuffer>(planes);
}

int UdmaBuf::exportFrameBuffers([[maybe_unused]]Camera *camera, Stream *stream,
				std::vector<std::unique_ptr<FrameBuffer>> *buffers)
{
	unsigned int count = stream->configuration().bufferCount;

	/** \todo: Support multiplanar allocations */
	size_t size = stream->configuration().frameSize;

	for (unsigned i = 0; i < count; ++i) {
		std::unique_ptr<FrameBuffer> buffer = createBuffer(size);
		if (!buffer) {
			LOG(UDMA, Error) << "Unable to create buffer";

			buffers->clear();
			return -EINVAL;
		}

		buffers->push_back(std::move(buffer));
	}

	return count;
}

} /* namespace libcamera */
