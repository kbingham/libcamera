/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * file_sink_turbojpeg.cpp - File Sink using libjpegturbo
 */

#include "file_sink_turbojpeg.h"

#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <libcamera/camera.h>

/* Only for tjFree until we have a custom deleter */
#include <turbojpeg.h>

using namespace libcamera;

FileSinkTurboJPEG::FileSinkTurboJPEG(const std::string &pattern)
	: pattern_(pattern)
{
	std::cerr << "Creating a FileSinkTurboJPEG with pattern: " << pattern << std::endl;
}

FileSinkTurboJPEG::~FileSinkTurboJPEG()
{
	for (auto &iter : mappedBuffers_) {
		void *memory = iter.second.first;
		unsigned int length = iter.second.second;
		munmap(memory, length);
	}
	mappedBuffers_.clear();
}

int FileSinkTurboJPEG::configure(const libcamera::CameraConfiguration &config)
{
	int ret = FrameSink::configure(config);
	if (ret < 0)
		return ret;

	/* \todo:
	 * Support more streams, dynamically createing compressors as required.
	 */
	if (config.size() > 1) {
		std::cerr << "Unsupported streams" << std::endl;
		return -1;
	}

	streamNames_.clear();
	for (unsigned int index = 0; index < config.size(); ++index) {
		const StreamConfiguration &cfg = config.at(index);
		streamNames_[cfg.stream()] = "stream" + std::to_string(index);
	}

	/* Configure against the first stream only at the moment. */
	ret = compressor_.configure(config.at(0));
	if (ret)
		std::cerr << "Failed to configure JPEG compressor" << std::endl;

	return ret;
}

void FileSinkTurboJPEG::mapBuffer(FrameBuffer *buffer)
{
	for (const FrameBuffer::Plane &plane : buffer->planes()) {
		void *memory = mmap(NULL, plane.length, PROT_READ, MAP_SHARED,
				    plane.fd.fd(), 0);

		mappedBuffers_[plane.fd.fd()] =
			std::make_pair(memory, plane.length);
	}
}

bool FileSinkTurboJPEG::consumeBuffer(const Stream *stream, FrameBuffer *buffer)
{
	std::string filename;
	size_t pos;
	int fd, ret = 0;
	Frame frame;

	filename = pattern_;
	pos = filename.find_first_of('#');
	if (pos != std::string::npos) {
		std::stringstream ss;
		ss << streamNames_[stream] << "-" << std::setw(6)
		   << std::setfill('0') << buffer->metadata().sequence;
		filename.replace(pos, 1, ss.str());
	}

	/*
	 * Prepare the FrameInfo.
	 * I find it distasteful that this dance is necessary...
	 * It should be easier to pass around all the properties necessary
	 * to access a buffer as a single object, without having to do all
	 * of these lookups on the FD.
	 */
	frame.buffer = buffer;
	for (unsigned int i = 0; i < buffer->planes().size(); ++i) {
		const FrameBuffer::Plane &plane = buffer->planes()[i];
		frame.memory[i].data = static_cast<const unsigned char *>(mappedBuffers_[plane.fd.fd()].first);
		frame.memory[i].length = plane.length;
	}

	TJJPEGImage jpeg;

	/* Try to compress first */
	ret = compressor_.compress(&frame, &jpeg);
	if (ret) {
		std::cerr << "Failed to compress frame: " << filename << std::endl;
		return ret;
	}

	fd = open(filename.c_str(), O_CREAT | O_WRONLY | (pos == std::string::npos ? O_APPEND : O_TRUNC),
		  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (fd == -1) {
		ret = errno;
		std::cerr << "Failed to open file: " << filename
			  << ": " << strerror(ret) << std::endl;
		return true;
	}

	/* Save the compressed image. */
	ret = ::write(fd, jpeg.data, jpeg.length);
	if (ret < 0) {
		ret = -errno;
		std::cerr << "write error: " << strerror(-ret)
			  << std::endl;
	} else if (ret != (int)jpeg.length) {
		std::cerr << "write error: only " << ret
			  << " bytes written instead of "
			  << jpeg.length << std::endl;
	}

	/*
	 * Make this work through an auto-deletion object.
	 * (or pre-allocated resources)
	 */
	tjFree(jpeg.data);

	close(fd);

	return true;
}
