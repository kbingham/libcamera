/*
 * vsp1.h
 */

#ifndef LIBCAMERA_SRC_VSP1_VSP1_H_
#define LIBCAMERA_SRC_VSP1_VSP1_H_

#include <libcamera/buffer.h>

#include "device_enumerator.h"
#include "media_device.h"

#include "v4l2_device.h"
#include "v4l2_subdevice.h"
#include "v4l2_videodevice.h"

using namespace libcamera;

class VSP1
{
public:
	VSP1()
		: media_(nullptr),
		  rpf_(nullptr), uds_(nullptr), sru_(nullptr), wpf_(nullptr),
		  rpfVideo_(nullptr), wpfVideo_(nullptr)
	{
	}

	~VSP1();
	int init();

	int allocateBuffers();

	int connectUDS();
	int connectSRU();
	void printMediaGraph();

protected:
	std::unique_ptr<DeviceEnumerator> enumerator_;
	std::shared_ptr<MediaDevice> media_;

	MediaEntity *rpf_;
	MediaEntity *uds_;
	MediaEntity *sru_;
	MediaEntity *wpf_;

	V4L2VideoDevice *rpfVideo_;
	V4L2VideoDevice *wpfVideo_;

	BufferPool input_; // Buffers cycled through the RPF
	BufferPool output_; // Buffers cycled through the WPF
};

#endif /* LIBCAMERA_SRC_VSP1_VSP1_H_ */
