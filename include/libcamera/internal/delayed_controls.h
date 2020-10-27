/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Raspberry Pi (Trading) Ltd.
 *
 * delayed_controls.h - Helper to deal with controls that are applied with a delay
 */
#ifndef __LIBCAMERA_INTERNAL_DELAYED_CONTROLS_H__
#define __LIBCAMERA_INTERNAL_DELAYED_CONTROLS_H__

#include <mutex>
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

	void reset(ControlList *controls = nullptr);

	bool push(const ControlList &controls);
	ControlList get(uint32_t sequence);

	void frameStart(uint32_t sequence);

private:
	class ControlInfo
	{
	public:
		ControlInfo()
			: updated(false)
		{
		}

		ControlInfo(const ControlValue &v)
			: value(v), updated(true)
		{
		}

		ControlValue value;
		bool updated;
	};

	static constexpr int listSize = 16;
	class ControlArray : public std::array<ControlInfo, listSize>
	{
	public:
		ControlInfo &operator[](unsigned int index)
		{
			return std::array<ControlInfo, listSize>::operator[](index % listSize);
		}

		const ControlInfo &operator[](unsigned int index) const
		{
			return std::array<ControlInfo, listSize>::operator[](index % listSize);
		}
	};

	using ControlsDelays = std::unordered_map<const ControlId *, unsigned int>;
	using ControlsValues = std::unordered_map<const ControlId *, ControlArray>;

	bool queue(const ControlList &controls);

	std::mutex lock_;

	V4L2Device *device_;
	ControlsDelays delays_;
	unsigned int maxDelay_;

	bool running_;
	uint32_t fistSequence_;

	uint32_t queueCount_;
	uint32_t writeCount_;
	ControlsValues ctrls_;
};

} /* namespace libcamera */

#endif /* __LIBCAMERA_INTERNAL_DELAYED_CONTROLS_H__ */
