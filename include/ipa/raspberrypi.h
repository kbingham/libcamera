/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Raspberry Pi Ltd.
 * Copyright (C) 2019, Google Inc.
 *
 * raspberrypi.h - Image Processing Algorithm interface for Raspberry Pi
 */
#ifndef __LIBCAMERA_IPA_INTERFACE_RASPBERRYPI_H__
#define __LIBCAMERA_IPA_INTERFACE_RASPBERRYPI_H__

enum RPiOperations {
	RPI_IPA_ACTION_V4L2_SET = 1,
	RPI_IPA_ACTION_PARAM_FILLED = 2,
	RPI_IPA_ACTION_METADATA = 3,
	RPI_IPA_EVENT_SIGNAL_STAT_BUFFER = 4,
	RPI_IPA_EVENT_QUEUE_REQUEST = 5,
};

#endif /* __LIBCAMERA_IPA_INTERFACE_RASPBERRYPI_H__ */
