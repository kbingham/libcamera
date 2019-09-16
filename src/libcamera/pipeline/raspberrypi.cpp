/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * raspberrypi.cpp - Pipeline handler for Raspberry Pi devices
 */

#include <libcamera/camera.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>

#include "camera_sensor.h"
#include "device_enumerator.h"
#include "log.h"
#include "media_device.h"
#include "pipeline_handler.h"
#include "utils.h"
#include "v4l2_controls.h"
#include "v4l2_videodevice.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(RPI)

class RPiCameraData : public CameraData
{
public:
	RPiCameraData(PipelineHandler *pipe)
		: CameraData(pipe), sensor_(nullptr), unicam_(nullptr),
		  isp_(nullptr)
	{
	}

	~RPiCameraData()
	{
		bayerBuffers_.destroyBuffers();
		delete sensor_;
		delete unicam_;
		delete isp_;
	}

	void sensorReady(Buffer *buffer);
	void ispOutputReady(Buffer *buffer);
	void ispCaptureReady(Buffer *buffer);

	CameraSensor *sensor_;
	V4L2VideoDevice *unicam_;
	V4L2M2MDevice *isp_;
	Stream stream_;

	BufferPool bayerBuffers_;
	std::vector<std::unique_ptr<Buffer>> rawBuffers_;
};

class RPiCameraConfiguration : public CameraConfiguration
{
public:
	RPiCameraConfiguration();

	Status validate() override;
};

class PipelineHandlerRPi : public PipelineHandler
{
public:
	PipelineHandlerRPi(CameraManager *manager);
	~PipelineHandlerRPi();

	CameraConfiguration *
	generateConfiguration(Camera *camera,
			      const StreamRoles &roles) override;
	int configure(Camera *camera,
		      CameraConfiguration *config) override;

	int allocateBuffers(Camera *camera,
			    const std::set<Stream *> &streams) override;
	int freeBuffers(Camera *camera,
			const std::set<Stream *> &streams) override;

	int start(Camera *camera) override;
	void stop(Camera *camera) override;

	int queueRequest(Camera *camera, Request *request) override;

	bool match(DeviceEnumerator *enumerator) override;

private:
	RPiCameraData *cameraData(const Camera *camera)
	{
		return static_cast<RPiCameraData *>(
			PipelineHandler::cameraData(camera));
	}

	std::shared_ptr<MediaDevice> unicam_;
	std::shared_ptr<MediaDevice> codec_;
};

RPiCameraConfiguration::RPiCameraConfiguration()
	: CameraConfiguration()
{
}

CameraConfiguration::Status RPiCameraConfiguration::validate()
{
	Status status = Valid;

	if (config_.empty())
		return Invalid;

	/* todo: Experiment with increased stream support through the ISP. */
	if (config_.size() > 1) {
		config_.resize(1);
		status = Adjusted;
	}

	StreamConfiguration &cfg = config_[0];

	/* todo: restrict to hardware capabilities. */

	cfg.bufferCount = 4;

	return status;
}

PipelineHandlerRPi::PipelineHandlerRPi(CameraManager *manager)
	: PipelineHandler(manager), unicam_(nullptr), codec_(nullptr)
{
}

PipelineHandlerRPi::~PipelineHandlerRPi()
{
	if (unicam_)
		unicam_->release();

	if (codec_)
		codec_->release();
}

CameraConfiguration *
PipelineHandlerRPi::generateConfiguration(Camera *camera,
					  const StreamRoles &roles)
{
	CameraConfiguration *config = new RPiCameraConfiguration();
	RPiCameraData *data = cameraData(camera);

	if (roles.empty())
		return config;

	StreamConfiguration cfg{};
	cfg.pixelFormat = V4L2_PIX_FMT_YUYV;
	cfg.size = { 1920, 1080 };

	LOG(RPI, Debug) << "Sensor Resolution is: " << data->sensor_->resolution().toString();

	cfg.bufferCount = 4;

	config->addConfiguration(cfg);

	config->validate();

	return config;
}

int PipelineHandlerRPi::configure(Camera *camera, CameraConfiguration *config)
{
	RPiCameraData *data = cameraData(camera);
	StreamConfiguration &cfg = config->at(0);
	uint32_t unicam_fourcc;
	int ret;

	Size sensorSize = { 1920, 1080 };
	Size outputSize = { 1920, 1088 };

	V4L2DeviceFormat format = {};
	format.size = sensorSize;

	LOG(RPI, Debug) << "Setting format to " << format.toString();

	ret = data->unicam_->setFormat(&format);
	if (ret)
		return ret;

	if (format.size != sensorSize) {
		LOG(RPI, Error)
			<< "Failed to set format on Video device: "
			<< format.toString();
		return -EINVAL;
	}

	format.size = outputSize;
	unicam_fourcc = format.fourcc;

	ret = data->isp_->output()->setFormat(&format);

	if (format.size != outputSize ||
	    format.fourcc != unicam_fourcc) {
		LOG(RPI, Error) << "Failed to set format on ISP output device: "
				<< format.toString();
		return -EINVAL;
	}

	/* Configure the ISP to generate the requested size and format. */
	format.size = cfg.size;
	format.fourcc = cfg.pixelFormat;

	ret = data->isp_->capture()->setFormat(&format);

	if (format.size != cfg.size ||
	    format.fourcc != cfg.pixelFormat) {
		LOG(RPI, Error)
			<< "Failed to set format on ISP capture device: "
			<< format.toString();
		return -EINVAL;
	}

	cfg.setStream(&data->stream_);

	return 0;
}

int PipelineHandlerRPi::allocateBuffers(Camera *camera,
					const std::set<Stream *> &streams)
{
	RPiCameraData *data = cameraData(camera);
	Stream *stream = *streams.begin();
	const StreamConfiguration &cfg = stream->configuration();
	int ret;

	/*
	 * Buffers are allocated on the camera, and the capture pad of the ISP:
	 *      unicam -> isp.output -> isp.capture -> Application
	 */

	/* Create a new intermediate buffer pool. */
	data->bayerBuffers_.createBuffers(cfg.bufferCount);

	/* Tie the unicam video buffers to the intermediate pool. */
	ret = data->unicam_->exportBuffers(&data->bayerBuffers_);
	if (ret)
		return ret;

	ret = data->isp_->output()->importBuffers(&data->bayerBuffers_);
	if (ret)
		return ret;

	/* Tie the stream buffers to the capture device of the ISP. */
	if (stream->memoryType() == InternalMemory)
		ret = data->isp_->capture()->exportBuffers(&stream->bufferPool());
	else
		ret = data->isp_->capture()->importBuffers(&stream->bufferPool());

	return ret;
}

int PipelineHandlerRPi::freeBuffers(Camera *camera,
				    const std::set<Stream *> &streams)
{
	RPiCameraData *data = cameraData(camera);
	int ret;

	ret = data->unicam_->releaseBuffers();
	if (ret)
		return ret;

	ret = data->isp_->output()->releaseBuffers();
	if (ret)
		return ret;

	ret = data->isp_->capture()->releaseBuffers();
	if (ret)
		return ret;

	data->bayerBuffers_.destroyBuffers();

	return ret;
}

int PipelineHandlerRPi::start(Camera *camera)
{
	RPiCameraData *data = cameraData(camera);
	int ret;

	data->rawBuffers_ = data->unicam_->queueAllBuffers();
	if (data->rawBuffers_.empty()) {
		LOG(RPI, Debug) << "Failed to queue unicam buffers";
		return -EINVAL;
	}

	LOG(RPI, Warning) << "Using hard-coded exposure/gain defaults";

	ControlList controls(data->sensor_->controls());

	controls.set(V4L2_CID_EXPOSURE, 1700);
	controls.set(V4L2_CID_ANALOGUE_GAIN, 180);
	ret = data->sensor_->setControls(&controls);
	if (ret) {
		LOG(RPI, Error) << "Failed to set controls";
		return ret;
	}

	ret = data->isp_->output()->streamOn();
	if (ret)
		return ret;

	ret = data->isp_->capture()->streamOn();
	if (ret)
		goto output_streamoff;

	ret = data->unicam_->streamOn();
	if (ret)
		goto capture_streamoff;

	return 0;

capture_streamoff:
	data->isp_->capture()->streamOff();
output_streamoff:
	data->isp_->output()->streamOff();

	return ret;
}

void PipelineHandlerRPi::stop(Camera *camera)
{
	RPiCameraData *data = cameraData(camera);

	data->isp_->capture()->streamOff();
	data->isp_->output()->streamOff();
	data->unicam_->streamOff();

	data->rawBuffers_.clear();
}

int PipelineHandlerRPi::queueRequest(Camera *camera, Request *request)
{
	RPiCameraData *data = cameraData(camera);
	Stream *stream = &data->stream_;

	Buffer *buffer = request->findBuffer(stream);
	if (!buffer) {
		LOG(RPI, Error)
			<< "Attempt to queue request with invalid stream";
		return -ENOENT;
	}

	int ret = data->isp_->capture()->queueBuffer(buffer);
	if (ret < 0)
		return ret;

	PipelineHandler::queueRequest(camera, request);

	return 0;
}

bool PipelineHandlerRPi::match(DeviceEnumerator *enumerator)
{
	DeviceMatch unicam("unicam");
	DeviceMatch codec("bcm2835-codec");

	/* The video node is also named unicam. */
	unicam.add("unicam");

	/* We explicitly need the ISP device from the MMAL codec driver. */
	codec.add("bcm2835-codec-isp-source");

	unicam_ = enumerator->search(unicam);
	if (!unicam_)
		return false;

	codec_ = enumerator->search(codec);
	if (!codec_)
		return false;

	unicam_->acquire();
	codec_->acquire();

	std::unique_ptr<RPiCameraData> data = utils::make_unique<RPiCameraData>(this);

	/* Locate and open the unicam video node. */
	data->unicam_ = new V4L2VideoDevice(unicam_->getEntityByName("unicam"));
	if (data->unicam_->open())
		return false;

	/* Locate the ISP M2M node */
	MediaEntity *isp = codec_->getEntityByName("bcm2835-codec-isp-source");
	if (!isp) {
		LOG(RPI, Error) << "Could not identify the ISP";
		return false;
	}

	data->isp_ = new V4L2M2MDevice(isp->deviceNode());
	if (data->isp_->open()) {
		LOG(RPI, Error) << "Could not open the ISP device";
		return false;
	}

	data->unicam_->bufferReady.connect(data.get(), &RPiCameraData::sensorReady);
	data->isp_->output()->bufferReady.connect(data.get(), &RPiCameraData::ispOutputReady);
	data->isp_->capture()->bufferReady.connect(data.get(), &RPiCameraData::ispCaptureReady);

	/* Identify the sensor */
	for (MediaEntity *entity : unicam_->entities()) {
		if (entity->function() == MEDIA_ENT_F_CAM_SENSOR) {
			data->sensor_ = new CameraSensor(entity);
			break;
		}
	}

	if (!data->sensor_)
		return false;

	if (data->sensor_->init())
		return false;

	/* Create and register the camera. */
	std::set<Stream *> streams{ &data->stream_ };
	std::shared_ptr<Camera> camera =
		Camera::create(this, data->sensor_->entity()->name(), streams);
	registerCamera(std::move(camera), std::move(data));

	return true;
}

void RPiCameraData::sensorReady(Buffer *buffer)
{
	/* \todo Handle buffer failures when state is set to BufferError. */
	if (buffer->status() == Buffer::BufferCancelled)
		return;

	/* Deliver the frame from the sensor to the ISP. */
	isp_->output()->queueBuffer(buffer);
}

void RPiCameraData::ispOutputReady(Buffer *buffer)
{
	/* \todo Handle buffer failures when state is set to BufferError. */
	if (buffer->status() == Buffer::BufferCancelled)
		return;

	/* Return a completed buffer from the ISP back to the sensor. */
	unicam_->queueBuffer(buffer);
}

void RPiCameraData::ispCaptureReady(Buffer *buffer)
{
	Request *request = buffer->request();

	pipe_->completeBuffer(camera_, request, buffer);
	pipe_->completeRequest(camera_, request);
}

REGISTER_PIPELINE_HANDLER(PipelineHandlerRPi);

} /* namespace libcamera */
