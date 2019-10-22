/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * raspberrypi.cpp - Pipeline handler for Raspberry Pi devices
 */

#include <ipa/raspberrypi.h>

#include <libcamera/camera.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>

#include "camera_sensor.h"
#include "device_enumerator.h"
#include "ipa_manager.h"
#include "log.h"
#include "media_device.h"
#include "pipeline_handler.h"
#include "utils.h"
#include "v4l2_controls.h"
#include "v4l2_videodevice.h"

/* RPi Definition, not yet in UAPI */
#define V4L2_META_FMT_STATS v4l2_fourcc('S', 'T', 'A', 'T')

namespace libcamera {

LOG_DEFINE_CATEGORY(RPI)

class RPiCameraData : public CameraData
{
public:
	RPiCameraData(PipelineHandler *pipe)
		: CameraData(pipe), sensor_(nullptr), unicam_(nullptr)
	{
	}

	~RPiCameraData()
	{
		bayerBuffers_.destroyBuffers();
		delete sensor_;
		delete unicam_;

		/* Perhaps move this to an ISP container class or struct */
		delete isp_.output_;
		delete isp_.capture0_;
		delete isp_.capture1_;
		delete isp_.stats_;
	}

	void sensorReady(Buffer *buffer);
	void ispOutputReady(Buffer *buffer);
	void ispCaptureReady(Buffer *buffer);
	void ispViewFinderReady(Buffer *buffer);
	void ispStatsReady(Buffer *buffer);

	int loadIPA();
	void queueFrameAction(unsigned int frame,
			      const IPAOperationData &action);

	void metadataReady(unsigned int frame, const ControlList &metadata);

	CameraSensor *sensor_;
	V4L2VideoDevice *unicam_;

	struct {
		V4L2VideoDevice *output_;
		V4L2VideoDevice *capture0_;
		V4L2VideoDevice *capture1_;
		V4L2VideoDevice *stats_;
	} isp_;

	Stream stream_;

	/* Sensor Capture buffers */
	BufferPool bayerBuffers_;
	std::vector<std::unique_ptr<Buffer>> rawBuffers_;

	/* View-finder buffers */
	BufferPool vfPool_;
	std::vector<std::unique_ptr<Buffer>> vfBuffers_;

	/* ISP statistics buffers */
	BufferPool statsPool_;
	std::vector<std::unique_ptr<Buffer>> statsBuffers_;
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
	std::shared_ptr<MediaDevice> isp_;
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
	: PipelineHandler(manager), unicam_(nullptr), isp_(nullptr)
{
}

PipelineHandlerRPi::~PipelineHandlerRPi()
{
	if (unicam_)
		unicam_->release();

	if (isp_)
		isp_->release();
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
	/*
	 * Output size failed when it was set to 1088, should ISP accept this,
	 * (even though, 1080 is what we actually want)? It depends on who
	 * allocates the buffers I guess...
	 *
	 * Who ever has a bigger size should allocate the buffers...
	 * All that said, this was a work around for the codec-isp for a bug
	 * which is fixed, so we need to rework this anyway. The only exception
	 * is we need to work out how to configure the FoV by using different
	 * sizes available on the sensor to produce different sizes at the end..
	 */
	Size outputSize = { 1920, 1080 };

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

	ret = data->isp_.output_->setFormat(&format);
	if (ret)
		return ret;

	if (format.size != outputSize ||
	    format.fourcc != unicam_fourcc) {
		LOG(RPI, Error) << "Failed to set format on ISP output device: "
				<< format.toString();
		return -EINVAL;
	}

	/* Configure the ISP to generate the requested size and format. */
	format.size = cfg.size;
	format.fourcc = cfg.pixelFormat;

	ret = data->isp_.capture0_->setFormat(&format);

	if (format.size != cfg.size ||
	    format.fourcc != cfg.pixelFormat) {
		LOG(RPI, Error)
			<< "Failed to set format on ISP capture device: "
			<< format.toString();
		return -EINVAL;
	}

	cfg.setStream(&data->stream_);

	/* Configure the ViewFinder ISP stream. */

	/* We must configure the viewfinder ISP channel even though we don't
	 * yet support multiple streams.
	 */

	/* Fixed (small) viewfinder stream for small buffers initially */
	format.size = { 320, 240 };
	format.fourcc = cfg.pixelFormat;

	ret = data->isp_.capture1_->setFormat(&format);
	if (ret) {
		LOG(RPI, Error)
			<< "Failed to set format on viewfinder ISP: "
			<< format.toString();
		return ret;
	}

	/* Configure the Stats buffer format */
	format.fourcc = V4L2_META_FMT_STATS;

	ret = data->isp_.stats_->setFormat(&format);
	if (ret) {
		LOG(RPI, Error)
			<< "Failed to set format on ISP stats node: "
			<< format.toString();
		return ret;
	}

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
	 * unicam -> isp.output |-> isp.capture0 -> Application
	 *			|-> isp.capture1 -> (VF Not enabled, loopback)
	 *			|-> isp.stats -> Internal IPA use only
	 */

	/* Create a new intermediate buffer pool. */
	data->bayerBuffers_.createBuffers(cfg.bufferCount);

	/* Tie the unicam video buffers to the intermediate pool. */
	ret = data->unicam_->exportBuffers(&data->bayerBuffers_);
	if (ret)
		return ret;

	ret = data->isp_.output_->importBuffers(&data->bayerBuffers_);
	if (ret)
		return ret;

	/* Create temporary internal buffers for the viewfinder stream */
	data->vfPool_.createBuffers(cfg.bufferCount);
	ret = data->isp_.capture1_->exportBuffers(&data->vfPool_);
	if (ret) {
		LOG(RPI, Error) << "Failed to create Viewfinder buffers";
		return ret;
	}

	/* Create internal buffers for the statistics stream */
	data->statsPool_.createBuffers(cfg.bufferCount);
	ret = data->isp_.stats_->exportBuffers(&data->statsPool_);
	if (ret) {
		LOG(RPI, Error) << "Failed to create Statistics buffers";
		return ret;
	}

	/* Tie the stream buffers to the capture device of the ISP. */
	if (stream->memoryType() == InternalMemory)
		ret = data->isp_.capture0_->exportBuffers(&stream->bufferPool());
	else
		ret = data->isp_.capture0_->importBuffers(&stream->bufferPool());

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

	ret = data->isp_.output_->releaseBuffers();
	if (ret)
		return ret;

	ret = data->isp_.capture0_->releaseBuffers();
	if (ret)
		return ret;

	data->bayerBuffers_.destroyBuffers();

	return ret;
}

int PipelineHandlerRPi::start(Camera *camera)
{
	RPiCameraData *data = cameraData(camera);
	ControlList controls(data->sensor_->controls());
	int ret;

	data->rawBuffers_ = data->unicam_->queueAllBuffers();
	if (data->rawBuffers_.empty()) {
		LOG(RPI, Debug) << "Failed to queue unicam buffers";
		return -EINVAL;
	}

	/* Queue internal viewfinder buffers. */
	data->vfBuffers_ = data->isp_.capture1_->queueAllBuffers();
	if (data->rawBuffers_.empty()) {
		LOG(RPI, Debug) << "Failed to queue viewfinder buffers";
		ret = -EINVAL;
		goto err;
	}

	/* Queue internal ISP buffers. */
	data->statsBuffers_ = data->isp_.stats_->queueAllBuffers();
	if (data->statsBuffers_.empty()) {
		LOG(RPI, Debug) << "Failed to queue internal ISP buffers";
		ret = -EINVAL;
		goto err;
	}

	LOG(RPI, Warning) << "Using hard-coded exposure/gain defaults";

	controls.set(V4L2_CID_EXPOSURE, 1700);
	controls.set(V4L2_CID_ANALOGUE_GAIN, 180);
	ret = data->sensor_->setControls(&controls);
	if (ret) {
		LOG(RPI, Error) << "Failed to set controls";
		goto err;
	}

	/* A clean (reduced line count) implementation below would be nice. */

	ret = data->isp_.output_->streamOn();
	if (ret)
		goto err;

	ret = data->isp_.capture0_->streamOn();
	if (ret)
		goto err;

	ret = data->isp_.capture1_->streamOn();
	if (ret)
		goto err;

	ret = data->isp_.stats_->streamOn();
	if (ret)
		goto err;

	ret = data->unicam_->streamOn();
	if (ret)
		goto err;

	return 0;

err:
	stop(camera);

	return ret;
}

void PipelineHandlerRPi::stop(Camera *camera)
{
	RPiCameraData *data = cameraData(camera);

	data->isp_.stats_->streamOff();
	data->isp_.capture1_->streamOff();
	data->isp_.capture0_->streamOff();
	data->isp_.output_->streamOff();
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

	int ret = data->isp_.capture0_->queueBuffer(buffer);
	if (ret < 0)
		return ret;

	PipelineHandler::queueRequest(camera, request);

	return 0;
}

bool PipelineHandlerRPi::match(DeviceEnumerator *enumerator)
{
	DeviceMatch unicam("unicam");
	DeviceMatch isp("bcm2835-isp");

	/* The video node is also named unicam. */
	unicam.add("unicam");

	isp.add("bcm2835-isp0-output0");
	isp.add("bcm2835-isp0-capture1"); /* Full */
	isp.add("bcm2835-isp0-capture2"); /* ViewFinder */
	isp.add("bcm2835-isp0-capture3"); /* Stats */

	unicam_ = enumerator->search(unicam);
	if (!unicam_)
		return false;

	isp_ = enumerator->search(isp);
	if (!isp_)
		return false;

	unicam_->acquire();
	isp_->acquire();

	std::unique_ptr<RPiCameraData> data = utils::make_unique<RPiCameraData>(this);

	/* Locate and open the unicam video node. */
	data->unicam_ = new V4L2VideoDevice(unicam_->getEntityByName("unicam"));
	if (data->unicam_->open())
		return false;

	/* Open the ISP video nodes. */
	data->isp_.output_ = new V4L2VideoDevice(isp_->getEntityByName("bcm2835-isp0-output0"));
	if (data->isp_.output_->open())
		return false;

	data->isp_.capture0_ = new V4L2VideoDevice(isp_->getEntityByName("bcm2835-isp0-capture1"));
	if (data->isp_.capture0_->open())
		return false;

	data->isp_.capture1_ = new V4L2VideoDevice(isp_->getEntityByName("bcm2835-isp0-capture2"));
	if (data->isp_.capture1_->open())
		return false;

	data->isp_.stats_ = new V4L2VideoDevice(isp_->getEntityByName("bcm2835-isp0-capture3"));
	if (data->isp_.stats_->open())
		return false;

	/* Wire up all the buffer connections */
	data->unicam_->bufferReady.connect(data.get(), &RPiCameraData::sensorReady);
	data->isp_.output_->bufferReady.connect(data.get(), &RPiCameraData::ispOutputReady);
	data->isp_.capture0_->bufferReady.connect(data.get(), &RPiCameraData::ispCaptureReady);
	data->isp_.capture1_->bufferReady.connect(data.get(), &RPiCameraData::ispViewFinderReady);
	data->isp_.stats_->bufferReady.connect(data.get(), &RPiCameraData::ispStatsReady);

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

	if (data->loadIPA()) {
		LOG(RPI, Error) << "Failed to load a suitable IPA library";
		return false;
	}

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
	isp_.output_->queueBuffer(buffer);
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

void RPiCameraData::ispViewFinderReady(Buffer *buffer)
{
	/* Simply requeue buffer for now */
	isp_.capture1_->queueBuffer(buffer);
}

void RPiCameraData::ispStatsReady(Buffer *buffer)
{
	/* Simply requeue buffer for now */
	isp_.stats_->queueBuffer(buffer);
}

int RPiCameraData::loadIPA()
{
	ipa_ = IPAManager::instance()->createIPA(pipe_, 1, 1);
	if (!ipa_)
		return -ENOENT;

	ipa_->queueFrameAction.connect(this,
				       &RPiCameraData::queueFrameAction);

	return 0;
}

void RPiCameraData::queueFrameAction(unsigned int frame,
				     const IPAOperationData &action)
{
	switch (action.operation) {
	case RPI_IPA_ACTION_V4L2_SET: {
#if 0 // disabled cargo-cult from RkISP1
		const ControlList &controls = action.controls[0];
		timeline_.scheduleAction(utils::make_unique<RkISP1ActionSetSensor>(frame,
										   sensor_,
										   controls));
#endif
		break;
	}
	case RPI_IPA_ACTION_PARAM_FILLED: {
#if 0 // disabled cargo-cult from RkISP1
		RPIFrameInfo *info = frameInfo_.find(frame);
		if (info)
			info->paramFilled = true;
#endif
		break;
	}
	case RPI_IPA_ACTION_METADATA:
		metadataReady(frame, action.controls[0]);
		break;
	default:
		LOG(RPI, Error) << "Unknown action " << action.operation;
		break;
	}
}

void RPiCameraData::metadataReady(unsigned int frame, const ControlList &metadata)
{
	LOG(RPI, Debug) << "Received some MetaData, but nothing I can do yet..";

#if 0 // disabled cargo-cult from RkISP1
	PipelineHandlerRPi *pipe = static_cast<PipelineHandlerRPi *>(pipe_);

	RkISP1FrameInfo *info = frameInfo_.find(frame);
	if (!info)
		return;

	info->request->metadata() = metadata;
	info->metadataProcessed = true;

	pipe->tryCompleteRequest(info->request);
#endif
}

REGISTER_PIPELINE_HANDLER(PipelineHandlerRPi);

} /* namespace libcamera */
