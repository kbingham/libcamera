/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020, Ideas on Board Oy
 *
 * frame_sink.cpp - Base Frame Sink Class
 */

#include "frame_sink.h"

FrameSink::~FrameSink()
{
}

int FrameSink::configure(const libcamera::CameraConfiguration &config)
{
	return 0;
}

void FrameSink::mapBuffer(libcamera::FrameBuffer *buffer)
{
}

int FrameSink::start()
{
	return 0;
}

int FrameSink::stop()
{
	return 0;
}
