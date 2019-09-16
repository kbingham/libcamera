/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * event_loop.cpp - Event loop support
 */

#include "event_loop.h"

#include <atomic>

#include <libcamera/event_dispatcher.h>

#include "thread.h"

/**
 * \file event_loop.h
 * \brief Event loop support
 */

namespace libcamera {

/**
 * \class EventLoop
 * \brief An event loop based on an EventDispatcher
 */

/**
 * \brief Create an event loop
 */
EventLoop::EventLoop()
	: exit_(true), exitCode_(-1)
{
}

/**
 * \brief Enter the event loop
 *
 * This method enter an event loop based on the event dispatcher instance for
 * the current thread, and blocks until the exit() method is called.
 *
 * \return The exit code passed to the exit() method
 */
int EventLoop::exec()
{
	exitCode_ = -1;
	exit_.store(false, std::memory_order_release);

	EventDispatcher *dispatcher = thread()->eventDispatcher();
	while (!exit_.load(std::memory_order_acquire))
		dispatcher->processEvents();

	return exitCode_;
}

/**
 * \brief Stop the event loop
 * \param[in] code The exit code
 *
 * This method interrupts the event loop started by the exec() method, causing
 * exec() to return \a code.
 */
void EventLoop::exit(int code)
{
	exitCode_ = code;
	exit_.store(true, std::memory_order_release);

	EventDispatcher *dispatcher = thread()->eventDispatcher();
	dispatcher->interrupt();
}

} /* namespace libcamera */
