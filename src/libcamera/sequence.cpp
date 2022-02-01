/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Ideas on Board Oy.
 *
 * sequence.cpp Sequence Number Observer
 */

#include <libcamera/base/log.h>

#include <libcamera/sequence.h>

/**
 * \file sequence.h
 * \brief Sequence number observer
 */

namespace libcamera {

/**
 * \class Sequence
 * \brief Sequence number tracking which expects monotonically incrementing
 *	  numbers
 *
 * The Sequence number observer is initialised with the first value it is given.
 * It will return a difference of the expected update value, against the newly
 * provided value - allowing the consumer to identify if a break in a sequence
 * has occured.
 */

/**
 * \brief Update the sequence observer with the latest value
 * \param seq The latest value for the sequence
 *
 * This function will update the state of the Sequence observer and identify any
 * non-monotonic increment or change that may occur and return the difference
 * from the expected update value.
 *
 * The sequence is initialised to the first value passed into \a update.
 *
 * \return The number of drops in the sequence that were detected
 */
__nodiscard int Sequence::update(unsigned int seq)
{
	if (!sequence_)
		sequence_ = seq - 1;

	/*
	 * Any update expects a single integer difference from
	 * the previous value.
	 */
	int diff = seq - sequence_.value() - 1;

	sequence_ = seq;

	return diff;
};

/**
 * \fn Sequence::reset
 * \brief Resets the sequence observer
 *
 * Re-initialises the sequence observer so that any known break in the monotonic
 * sequence is not reported.
 */

} /* namespace libcamera */
