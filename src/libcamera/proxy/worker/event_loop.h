/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * event_loop.h - Event loop support
 */
#ifndef __LIBCAMERA_PROXY_WORKER_EVENT_LOOP_H__
#define __LIBCAMERA_PROXY_WORLER_EVENT_LOOP_H__

#include <atomic>

#include <libcamera/object.h>

namespace libcamera {

class EventLoop final : public Object
{
public:
	EventLoop();

	int exec();
	void exit(int code = 0);

private:
	std::atomic<bool> exit_;
	int exitCode_;
};

} /* namespace libcamera */

#endif /* __LIBCAMERA_PROXY_WORKER_EVENT_LOOP_H__ */
