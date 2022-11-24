/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * fake.cpp - Pipeline handler for fake cameras
 */

#include <algorithm>
#include <fcntl.h>
#include <gbm.h>
#include <iomanip>
#include <limits.h>
#include <memory>
#include <set>
#include <unistd.h>
#include <vector>

#include <libcamera/base/log.h>
#include <libcamera/base/utils.h>

#include <libcamera/camera_manager.h>
#include <libcamera/control_ids.h>
#include <libcamera/controls.h>
#include <libcamera/formats.h>
#include <libcamera/property_ids.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>

#include "libcamera/internal/camera.h"
#include "libcamera/internal/device_enumerator.h"
#include "libcamera/internal/formats.h"
#include "libcamera/internal/framebuffer.h"
#include "libcamera/internal/mapped_framebuffer.h"
#include "libcamera/internal/media_device.h"
#include "libcamera/internal/pipeline_handler.h"
#include "libcamera/internal/udma_allocator.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(Fake)

uint64_t CurrentTimestamp()
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
		LOG(Fake, Error) << "Get clock time fails";
		return 0;
	}

	return ts.tv_sec * 1'000'000'000LL + ts.tv_nsec;
}

static const ControlInfoMap::Map FakeControls = {
	{ &controls::draft::PipelineDepth, ControlInfo(2, 3) },
};

class FakeCameraData : public Camera::Private
{
public:
	struct Resolution {
		Size size;
		std::vector<int> frame_rates;
		std::vector<PixelFormat> formats;
	};

	FakeCameraData(PipelineHandler *pipe)
		: Camera::Private(pipe)
	{
	}

	std::vector<Resolution> supportedResolutions_;

	Stream stream_;

	bool started_ = false;
};

class FakeCameraConfiguration : public CameraConfiguration
{
public:
	static constexpr unsigned int kBufferCount = 4; // 4~6
	static constexpr unsigned int kMaxStreams = 1;

	FakeCameraConfiguration(FakeCameraData *data);

	Status validate() override;

private:
	/*
	 * The FakeCameraData instance is guaranteed to be valid as long as the
	 * corresponding Camera instance is valid. In order to borrow a
	 * reference to the camera data, store a new reference to the camera.
	 */
	const FakeCameraData *data_;
};

class PipelineHandlerFake : public PipelineHandler
{
public:
	PipelineHandlerFake(CameraManager *manager);

	std::unique_ptr<CameraConfiguration> generateConfiguration(Camera *camera,
								   const StreamRoles &roles) override;
	int configure(Camera *camera, CameraConfiguration *config) override;

	int exportFrameBuffers(Camera *camera, Stream *stream,
			       std::vector<std::unique_ptr<FrameBuffer>> *buffers) override;

	int start(Camera *camera, const ControlList *controls) override;
	void stopDevice(Camera *camera) override;

	int queueRequestDevice(Camera *camera, Request *request) override;

	bool match(DeviceEnumerator *enumerator) override;

private:
	FakeCameraData *cameraData(Camera *camera)
	{
		return static_cast<FakeCameraData *>(camera->_d());
	}

	int registerCameras();

	static bool registered_;
};

bool PipelineHandlerFake::registered_ = false;

FakeCameraConfiguration::FakeCameraConfiguration(FakeCameraData *data)
	: CameraConfiguration()
{
	data_ = data;
}

CameraConfiguration::Status FakeCameraConfiguration::validate()
{
	Status status = Valid;

	if (config_.empty())
		return Invalid;

	Size maxSize(1920, 1080);

	/* Cap the number of entries to the available streams. */
	if (config_.size() > kMaxStreams) {
		config_.resize(kMaxStreams);
		status = Adjusted;
	}

	for (unsigned int i = 0; i < config_.size(); ++i) {
		const StreamConfiguration originalCfg = config_[i];
		StreamConfiguration *cfg = &config_[i];

		LOG(Fake, Debug) << "Validating stream: " << config_[i].toString();

		cfg->pixelFormat = formats::ABGR8888;

		const PixelFormatInfo &info = PixelFormatInfo::info(cfg->pixelFormat);

		cfg->bufferCount = FakeCameraConfiguration::kBufferCount;
		cfg->stride = info.stride(cfg->size.width, 0, 1);
		cfg->frameSize = info.frameSize(cfg->size, 1);

		if (cfg->pixelFormat != originalCfg.pixelFormat ||
		    cfg->size != originalCfg.size) {
			LOG(Fake, Debug)
				<< "Stream " << i << " configuration adjusted to "
				<< cfg->toString();
			status = Adjusted;
		}
	}

	return status;
}

PipelineHandlerFake::PipelineHandlerFake(CameraManager *manager)
	: PipelineHandler(manager)
{
	// TODO: read the fake hal spec file.
}

std::unique_ptr<CameraConfiguration> PipelineHandlerFake::generateConfiguration(Camera *camera,
										const StreamRoles &roles)
{
	FakeCameraData *data = cameraData(camera);
	std::unique_ptr<FakeCameraConfiguration> config = std::make_unique<FakeCameraConfiguration>(data);

	if (roles.empty())
		return config;

	Size minSize, sensorResolution;
	for (const auto &resolution : data->supportedResolutions_) {
		if (minSize.isNull() || minSize > resolution.size)
			minSize = resolution.size;

		sensorResolution = std::max(sensorResolution, resolution.size);
	}

	for (const StreamRole role : roles) {
		std::map<PixelFormat, std::vector<SizeRange>> streamFormats;
		unsigned int bufferCount;
		PixelFormat pixelFormat;
		Size size;

		switch (role) {
		case StreamRole::StillCapture:
			size = sensorResolution;
			pixelFormat = formats::BGRA8888;
			bufferCount = FakeCameraConfiguration::kBufferCount;
			streamFormats[pixelFormat] = { { minSize, size } };

			break;

		case StreamRole::Raw: {
			// TODO: check
			pixelFormat = formats::BGRA8888;
			size = sensorResolution;
			bufferCount = FakeCameraConfiguration::kBufferCount;
			streamFormats[pixelFormat] = { { minSize, size } };

			break;
		}

		case StreamRole::Viewfinder:
		case StreamRole::VideoRecording: {
			/*
			 * Default viewfinder and videorecording to 1280x720,
			 * capped to the maximum sensor resolution and aligned
			 * to the ImgU output constraints.
			 */
			size = sensorResolution;
			pixelFormat = formats::BGRA8888;
			bufferCount = FakeCameraConfiguration::kBufferCount;
			streamFormats[pixelFormat] = { { minSize, size } };

			break;
		}

		default:
			LOG(Fake, Error)
				<< "Requested stream role not supported: " << role;
			return nullptr;
		}

		StreamFormats formats(streamFormats);
		StreamConfiguration cfg(formats);
		cfg.size = size;
		cfg.pixelFormat = pixelFormat;
		cfg.bufferCount = bufferCount;
		config->addConfiguration(cfg);
	}

	if (config->validate() == CameraConfiguration::Invalid)
		return {};

	return config;
}

int PipelineHandlerFake::configure(Camera *camera, CameraConfiguration *config)
{
	FakeCameraData *data = cameraData(camera);
	StreamConfiguration &cfg = config->at(0);

	cfg.setStream(&data->stream_);

	return 0;
}

int PipelineHandlerFake::exportFrameBuffers(Camera *camera, Stream *stream,
					    std::vector<std::unique_ptr<FrameBuffer>> *buffers)
{
	UdmaBuf allocator;
	if (!allocator.isValid())
		return -ENODEV;

	return allocator.exportFrameBuffers(camera, stream, buffers);
}

int PipelineHandlerFake::start(Camera *camera, [[maybe_unused]] const ControlList *controls)
{
	FakeCameraData *data = cameraData(camera);
	data->started_ = true;

	return 0;
}

void PipelineHandlerFake::stopDevice(Camera *camera)
{
	FakeCameraData *data = cameraData(camera);

	data->started_ = false;
}

static void FillBuffer(const FrameBuffer *buffer, int idx) {
	MappedFrameBuffer mfb(buffer, MappedFrameBuffer::MapFlag::ReadWrite);

	if (mfb.isValid()) {
		MappedBuffer::Plane plane = mfb.planes()[0];
		uint8_t *data = plane.data();

		for (unsigned int x = 0; x < plane.size(); x += 4) {
			/* R */ data[x + 0] = 255.0 * ((float)x / plane.size());
			/* G */ data[x + 1] = 255.0 - (255.0 * ((float)x / plane.size()));
			/* B */ data[x + 2] = 255.0 * ((float)idx / 4);
			/* A */ data[x + 3] = 0x00;
		}
	}
}

int PipelineHandlerFake::queueRequestDevice(Camera *camera, Request *request)
{
	if (!camera)
		return -EINVAL;

	static unsigned int filled = 0;
	for (auto it : request->buffers()) {
		if (filled < FakeCameraConfiguration::kBufferCount) {
			FillBuffer(it.second, filled);
			filled++;
		}

		/* Run exceptionally slowly to view each individual buffer. */
		if (camera->_d()->isRunning())
			sleep(1);

		completeBuffer(request, it.second);
	}

	// TODO: request.metadata()
	request->metadata().set(controls::SensorTimestamp, CurrentTimestamp());
	completeRequest(request);

	return 0;
}

bool PipelineHandlerFake::match(DeviceEnumerator *enumerator)
{
	// TODO: exhaust all devices in |enumerator|.
	if (!enumerator)
		LOG(Fake, Info) << "Invalid enumerator";

	if (registered_)
		return false;

	registered_ = true;
	return registerCameras() == 0;
}

/**
 * \brief Initialise ImgU and CIO2 devices associated with cameras
 *
 * Initialise the two ImgU instances and create cameras with an associated
 * CIO2 device instance.
 *
 * \return 0 on success or a negative error code for error or if no camera
 * has been created
 * \retval -ENODEV no camera has been created
 */
int PipelineHandlerFake::registerCameras()
{
	std::unique_ptr<FakeCameraData> data =
		std::make_unique<FakeCameraData>(this);
	std::set<Stream *> streams = {
		&data->stream_,
	};

	// TODO: Read from config or from IPC.
	// TODO: Check with Han-lin: Can this function be called more than once?
	data->supportedResolutions_.resize(2);
	data->supportedResolutions_[0].size = Size(1920, 1080);
	data->supportedResolutions_[0].frame_rates.push_back(30);
	data->supportedResolutions_[0].formats.push_back(formats::BGRA8888);
	data->supportedResolutions_[1].size = Size(1280, 720);
	data->supportedResolutions_[1].frame_rates.push_back(30);
	data->supportedResolutions_[1].frame_rates.push_back(60);
	data->supportedResolutions_[1].frame_rates.push_back(120);
	data->supportedResolutions_[1].formats.push_back(formats::BGRA8888);

	// TODO: Assign different locations for different cameras based on config.
	data->properties_.set(properties::Location, properties::CameraLocationFront);
	data->properties_.set(properties::PixelArrayActiveAreas, { Rectangle(Size(1920, 1080)) });

	// TODO: Set FrameDurationLimits based on config.
	ControlInfoMap::Map controls = FakeControls;
	int64_t min_frame_duration = 30, max_frame_duration = 60;
	controls[&controls::FrameDurationLimits] = ControlInfo(min_frame_duration, max_frame_duration);
	data->controlInfo_ = ControlInfoMap(std::move(controls), controls::controls);

	std::shared_ptr<Camera> camera =
		Camera::create(std::move(data), "virtual" /* cameraId */, streams);

	manager_->addCamera(std::move(camera), {});

	return 0;
}

REGISTER_PIPELINE_HANDLER(PipelineHandlerFake)

} /* namespace libcamera */
