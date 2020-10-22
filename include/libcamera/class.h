/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * class.h - class helpers
 */
#ifndef __LIBCAMERA_CLASS_H__
#define __LIBCAMERA_CLASS_H__

/**
 * \brief Delete the copy constructor and assignment operator.
 */
#define DELETE_COPY_AND_ASSIGN(klass)   \
	/* copy constructor. */         \
	klass(const klass &) = delete;  \
	/* copy assignment operator. */ \
	klass &operator=(const klass &) = delete

/**
 * \brief Delete the move construtor and assignment operator.
 */
#define DELETE_MOVE_AND_ASSIGN(klass)   \
	/* move constructor. */         \
	klass(klass &&) = delete;       \
	/* move assignment operator. */ \
	klass &operator=(klass &&) = delete

/**
 * \brief Delete all copy and move constructors, and assignment operators.
 */
#define DELETE_COPY_MOVE_AND_ASSIGN(klass) \
	DELETE_COPY_AND_ASSIGN(klass);     \
	DELETE_MOVE_AND_ASSIGN(klass)

#endif /* __LIBCAMERA_CLASS_H__ */
