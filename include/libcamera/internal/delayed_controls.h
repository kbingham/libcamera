/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Raspberry Pi (Trading) Ltd.
 *
 * delayed_controls.h - Helper to deal with controls that are applied with a delay
 */
#ifndef __LIBCAMERA_INTERNAL_DELAYED_CONTROLS_H__
#define __LIBCAMERA_INTERNAL_DELAYED_CONTROLS_H__

#include <stdint.h>
#include <unordered_map>

#include <libcamera/controls.h>

namespace libcamera {

class V4L2Device;

class DelayedControls
{
public:
	DelayedControls(V4L2Device *device,
			const std::unordered_map<uint32_t, unsigned int> &delays);

	void reset();

	bool push(const ControlList &controls);
	ControlList get(uint32_t sequence);

	void applyControls(uint32_t sequence);

private:
	class Info
	{
	public:
		Info()
			: updated(false)
		{
		}

		Info(const ControlValue &v)
			: value(v), updated(true)
		{
		}

		ControlValue value;
		bool updated;
	};

	/* \todo: Make the listSize configurable at instance creation time. */
	static constexpr int listSize = 16;
	class ControlRingBuffer : public std::array<Info, listSize>
	{
	public:
		Info &operator[](unsigned int index)
		{
			return std::array<Info, listSize>::operator[](index % listSize);
		}

		const Info &operator[](unsigned int index) const
		{
			return std::array<Info, listSize>::operator[](index % listSize);
		}
	};

	bool queue(const ControlList &controls);

	V4L2Device *device_;
	std::unordered_map<const ControlId *, unsigned int> delays_;
	unsigned int maxDelay_;

	bool running_;
	uint32_t firstSequence_;

	uint32_t queueCount_;
	uint32_t writeCount_;
	std::unordered_map<const ControlId *, ControlRingBuffer> values_;
};

} /* namespace libcamera */

#endif /* __LIBCAMERA_INTERNAL_DELAYED_CONTROLS_H__ */
