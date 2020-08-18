.. SPDX-License-Identifier: CC-BY-SA-4.0

Pipeline Handler Writers Guide
==============================

Pipeline handlers are the abstraction layer for device-specific hardware
configuration. They access and control hardware through the V4L2 and Media
Controller kernel interfaces, and implement an internal API to control the ISP
and capture components of a pipeline directly.

Prerequisite knowledge: system architecture
-------------------------------------------

A pipeline handler configures and manages the image acquisition and
transformation pipeline realized by specialized system peripherals combined with
an image source connected to the system through a data and control bus. The
presence, number and characteristics of them vary depending on the system design
and the product integration of the target platform.

System components can be classified in three macro-categories:

- Input ports: interfaces to external devices, usually image sensors,
  which transfer data from the physical bus to locations accessible by other
  system peripherals. An input port needs to be configured according to the
  input image format and size and could optionally apply basic transformations
  on the received images, most typically cropping/scaling and some formats
  conversion. The industry standard for the system typically targeted by
  libcamera is to have receivers compliant with the MIPI CSI-2 specifications,
  implemented on a compatible physical layer such as MIPI D-PHY or MIPI C-PHY.
  Other design are possible but less common, such as LVDS or the legacy BT.601
  and BT.656 parallel protocols.
  .. TODO: Insert references to the open CSI-2 (and other) specification.

- Image Signal Processor (ISP): A specialized media processor which applies
  digital transformations on image streams. ISPs can be integrated as part of
  the SoC as a memory interfaced system peripheral or packaged as stand-alone
  chips connected to the application processor through a bus. Most hardware used
  by libcamera makes use of in-system ISP designs but pipelines can equally
  support external ISP chips or be instrumented to use other system resources
  such as a GPU or an FPGA IP block. ISPs expose a software programming
  interface that allows the configuration of multiple processing blocks which
  form an "Image Transformation Pipeline". An ISP usually produces 'processed'
  image streams along with the metadata describing the processing steps which
  have been applied to generate the output frames.

- Camera Sensor: digital component that integrates an image sensor with control
  electronics and usually a lens. It interfaces to the SoC image receiver ports
  and is programmed to produce images in a format and size suitable for the
  current system configuration. Complex camera modules can integrate on-board
  ISP or DSP chips and process images before delivering them to the system. Most
  systems with a dedicated ISP processor usually integrate camera sensors which
  produce images in Raw Bayer format and defer processing to it.

It is responsibility of the pipeline handler to interface with these (and
possibly other) components of the system and implement the following
functionalities:

- Detect and register camera devices available in the system with an associated
  set of image streams.

- Configure the image acquisition and processing pipeline by assigning the
  system resources (memory, shared components, etc.) to satisfy the application
  requested configuration.

- Start and the stop the image acquisition and processing sessions.

- Apply to the hardware devices the configuration settings computed by
  applications and image processing algorithms integrated in libcamera.

- Notify the availability of new images to applications and deliver them in the
  in the correct locations.

Prerequisite knowledge: libcamera architecture
----------------------------------------------

A pipeline handler uses most of the following libcamera classes to realize the
above described functionalities. Below is a brief overview of each of those:

.. TODO: Convert to sphinx refs

-  `MediaDevice <http://libcamera.org/api-html/classlibcamera_1_1MediaDevice.html>`_:
   Instances of this class are associated with a kernel media controller
   device and its connected objects.

   .. TODO: Reference to the Media Device API (possibly with versioning requirements)

-  `DeviceEnumerator <http://libcamera.org/api-html/classlibcamera_1_1DeviceEnumerator.html>`_:
   Enumerates all media devices attached to the system and the media entities
   registered with it, by creating instances of the ``MediaDevice`` class and
   by storing them.

-  `DeviceMatch <http://libcamera.org/api-html/classlibcamera_1_1DeviceMatch.html>`_:
   Describes a media device search pattern using entity names, or other
   properties.

-  `V4L2VideoDevice <http://libcamera.org/api-html/classlibcamera_1_1V4L2VideoDevice.html>`_:
   Models an instance of a V4L2 video device constructed with the path to a V4L2
   video device node.

-  `V4L2SubDevice <http://libcamera.org/api-html/classlibcamera_1_1V4L2Subdevice.html>`_:
   Provides an API to the sub-devices that model the hardware components of a
   V4L2 device.

-  `CameraSensor <http://libcamera.org/api-html/classlibcamera_1_1CameraSensor.html>`_:
   Abstracts camera sensor handling by hiding the details of the V4L2 subdevice
   kernel API and caching sensor information.

-  `CameraData <http://libcamera.org/api-html/classlibcamera_1_1CameraData.html>`_:
   Represents device-specific data a pipeline handler associates to a Camera
   instance.

-  `StreamConfiguration <http://libcamera.org/api-html/structlibcamera_1_1StreamConfiguration.html>`_:
   Models the current configuration of an image stream produced by the camera by
   reporting its format and sizes.

-  `CameraConfiguration <http://libcamera.org/api-html/classlibcamera_1_1CameraConfiguration.html>`_:
   Represents the current configuration of a camera, which includes a list of
   stream configurations for each active stream in a capture session. When
   validated, it is applied to the camera.

-  `IPAInterface <http://libcamera.org/api-html/classlibcamera_1_1IPAInterface.html>`_:
   The interface to the Image Processing Algorithm (IPA) module which performs
   the computation of the image processing pipeline tuning parameters.
   .. TODO: refer to the IPA guide

-  `ControlList <http://libcamera.org/api-html/classlibcamera_1_1ControlList.html>`_:
   A list of control items, indexed by Control<> instances or by numerical index
   which contains values used by application and IPA to change parameters of
   image streams, used to return to applications and share with IPA the metadata
   associated with the captured images, and to advertise the immutable camera
   characteristic enumerated at system initialization time.

Creating a PipelineHandler
--------------------------

This guide walks through the steps to create a simple pipeline handler
called “Vivid” that supports the `V4L2 Virtual Video Test Driver
(vivid) <https://www.kernel.org/doc/html/latest/admin-guide/media/vivid.html>`_.

To use the vivid test driver, you first need to check that the vivid kernel
module is loaded, for example with the ``modprobe vivid`` command.

Create the skeleton file structure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To add a new pipeline handler, create a directory to hold the pipeline code in
the *src/libcamera/pipeline/* directory that matches the name of the pipeline
(in this case *vivid*). Inside the new directory add a *meson.build* file that
integrates with the libcamera build system, and a *vivid.cpp* file that matches
the name of the pipeline.

In the *meson.build* file, add the *vivid.cpp* file as a build source for
libcamera by adding it to the global meson ``libcamera_sources`` variable:

.. code-block:: none

   # SPDX-License-Identifier: CC0-1.0

   libcamera_sources += files([
       'vivid.cpp',
   ])

Users of libcamera can selectively enable pipelines while building libcamera
using the ``pipelines`` option.

For example, to enable only the IPU3, UVC, and VIVID pipelines, specify them as
a comma separated list with ``-Dpipelines`` when generating a build directory:

.. code-block:: shell

    meson build -Dpipelines=ipu3,uvcvideo,vivid

`Read the Meson build configuration documentation
<https://mesonbuild.com/Configuring-a-build-directory.html>`_ for more
information.

To add the new pipeline handler to this list of options, add its directory name
to the libcamera build options in the top level _meson_options.txt_.

.. code-block:: none

   option('pipelines',
           type : 'array',
           choices : ['ipu3', 'raspberrypi', 'rkisp1', 'simple', 'uvcvideo', 'vimc', 'vivid'],
           description : 'Select which pipeline handlers to include')


In *vivid.cpp* add the pipeline handler to the ``libcamera`` namespace, define a
`PipelineHandler
<http://libcamera.org/api-html/classlibcamera_1_1PipelineHandler.html>`_ derived
class named PipelineHandlerVivid, and add stub methods for the overridden class
member.

.. code-block:: cpp

   namespace libcamera {
   class PipelineHandlerVivid : public PipelineHandler
   {
   public:
          PipelineHandlerVivid(CameraManager *manager);

          CameraConfiguration *generateConfiguration(Camera *camera,
          const StreamRoles &roles) override;
          int configure(Camera *camera, CameraConfiguration *config) override;

          int exportFrameBuffers(Camera *camera, Stream *stream,
          std::vector<std::unique_ptr<FrameBuffer>> *buffers) override;

          int start(Camera *camera) override;
          void stop(Camera *camera) override;

          int queueRequestDevice(Camera *camera, Request *request) override;

          bool match(DeviceEnumerator *enumerator) override;
   };

   PipelineHandlerVivid::PipelineHandlerVivid(CameraManager *manager)
          : PipelineHandler(manager)
   {
   }

   CameraConfiguration *PipelineHandlerVivid::generateConfiguration(Camera *camera,
                                                                    const StreamRoles &roles)
   {
          return nullptr;
   }

   int PipelineHandlerVivid::configure(Camera *camera, CameraConfiguration *config)
   {
          return -1;
   }

   int PipelineHandlerVivid::exportFrameBuffers(Camera *camera, Stream *stream,
                                                std::vector<std::unique_ptr<FrameBuffer>> *buffers)
   {
          return -1;
   }

   int PipelineHandlerVivid::start(Camera *camera)
   {
          return -1;
   }

   void PipelineHandlerVivid::stop(Camera *camera)
   {
   }

   int PipelineHandlerVivid::queueRequestDevice(Camera *camera, Request *request)
   {
          return -1;
   }

   bool PipelineHandlerVivid::match(DeviceEnumerator *enumerator)
   {
          return false;
   }
   } /* namespace libcamera */

You must register the ``PipelineHandler`` subclass with the pipeline handler
factory using the `REGISTER_PIPELINE_HANDLER
<http://libcamera.org/api-html/pipeline__handler_8h.html>`_ macro which
registers it and creates a global symbol to reference the class and make it
available to try and match devices.

Add the following before the closing curly bracket of the namespace declaration:

.. code-block:: cpp

   REGISTER_PIPELINE_HANDLER(PipelineHandlerVivid);

For debugging and testing a pipeline handler during development, you can define
a log message category for the pipeline handler. The ``LOG_DEFINE_CATEGORY``
macro and ``LIBCAMERA_LOG_LEVELS`` environment variable help you use the
`inbuilt libcamera logging infrastructure
<http://libcamera.org/api-html/log_8h.html>`_ that allow for the inspection of
internal operations in a user-configurable way.

Add the following before the ``PipelineHandlerVivid`` class declaration:

.. code-block:: cpp

   LOG_DEFINE_CATEGORY(VIVID)

At this point you need the following includes for logging and pipeline handler
features:

.. code-block:: cpp

   #include "libcamera/internal/log.h"
   #include "libcamera/internal/pipeline_handler.h"

Run:

.. code-block:: shell

   meson build
   ninja -C build install


To build the libcamera code base, and confirm that the build system found the
new pipeline handler by running:

.. code-block:: shell

   LIBCAMERA_LOG_LEVELS=Pipeline:0 ./build/src/cam/cam -l

And you should see output like the below:

.. code-block:: shell

    DEBUG Pipeline pipeline_handler.cpp:680 Registered pipeline handler "PipelineHandlerVivid"

Matching devices
~~~~~~~~~~~~~~~~

Each pipeline handler registered in libcamera gets tested against the current
system configuration, by matching a ``DeviceMatch`` with the system
``DeviceEnumerator``. A successful match makes sure all the requested components
have been registered in the system and allows the pipeline handler to be
initialized.

The main entry point of a pipeline handler is the `match
<http://libcamera.org/api-html/classlibcamera_1_1DeviceMatch.html>`_
class member function. When the ``CameraManager`` is started (using the `start
<http://libcamera.org/api-html/classlibcamera_1_1CameraManager.html#a49e322880a2a26013bb0076788b298c5>`_
method), all the registered pipeline handlers are iterated and their ``match``
function called with an enumerator of all devices it found on a system.

The match method should identify if there are suitable devices available in the
``DeviceEnumerator`` which the pipeline supports, returning ``true`` if it
matches a device, and ``false`` if it does not. To do this, construct the
`DeviceMatch
<http://libcamera.org/api-html/classlibcamera_1_1DeviceMatch.html>`_ class with
the name of the ``MediaController`` device to match. You can specify the search
further by adding specific media entities to the search using the ``.add()``
method on the DeviceMatch.

This example uses search patterns that match vivid, but you should change this
value to suit your device identifier.

Replace the contents of the ``PipelineHandlerVivid::match`` method with the
following:

.. code-block:: cpp

   DeviceMatch dm("vivid");
   dm.add("vivid-000-vid-cap");
   return false; // Prevent infinite loops for now

With the device matching criteria defined, attempt to acquire exclusive access
to the matching media controller device with the `acquireMediaDevice
<http://libcamera.org/api-html/classlibcamera_1_1PipelineHandler.html#a77e424fe704e7b26094164b9189e0f84>`_
method. If the method attempts to acquire a device it has already matched, it
returns ``false``.

Add the following below ``dm.add("vivid-000-vid-cap");``:

.. code-block:: cpp

   MediaDevice *media = acquireMediaDevice(enumerator, dm);
   if (!media)
           return false;

The pipeline handler now needs an additional include. Add the following to the
existing include block for device enumeration functionality:

.. code-block:: cpp

   #include "libcamera/internal/device_enumerator.h"

At this stage, you should test that the pipeline handler can successfully match
the devices, but have not yet added any code to create a Camera which libcamera
reports to applications.

As a temporary validation step, add a debug print with ``LOG(VIVID, Debug) <<
"Vivid Device Identified";`` before the closing ``return false; // Prevent
infinite loops for now`` in the ``PipelineHandlerVivid::match`` method for when
when the pipeline handler successfully matches the ``MediaDevice`` and
``MediaEntity`` names.

Test that the pipeline handler matches and finds a device by rebuilding, and
running

.. code-block:: shell

   LIBCAMERA_LOG_LEVELS=Pipeline,VIVID:0 ./build/src/cam/cam -l

And you should see output like the below:

.. code-block:: shell

    DEBUG VIVID vivid.cpp:74 Vivid Device Identified

Creating camera devices
~~~~~~~~~~~~~~~~~~~~~~~

If the pipeline handler successfully matches with the system it is running on,
it can proceed to initialization, by creating all the required instances of the
``V4L2VideoDevice``, ``V4L2Subdevice`` and ``CameraSensor`` hardware abstraction
classes, optionally initialize the IPA module and then proceed to the creation
of the Camera devices.

To each registered camera a set of image streams has to be associated. An image
``Stream`` represents a sequence of images and data of known size and format,
stored in application-accessible memory locations. Typical examples of streams
are the ISP processed outputs and the raw images captured at the receivers port
output.

Each Camera has instance-specific data represented by using the `CameraData
<http://libcamera.org/api-html/classlibcamera_1_1CameraData.html>`_ class, which
you extend for the specific needs of the pipeline handler.

Define a ``CameraData`` derived class ``VividCameraData()`` and initialize the
base ``CameraData`` class using the base ``PipelineHandler`` pointer.

Add the following code after the ``LOG_DEFINE_CATEGORY(VIVID)`` line:

.. code-block:: cpp

   class VividCameraData : public CameraData
   {
   public:
          VividCameraData(PipelineHandler *pipe, MediaDevice *media)
                : CameraData(pipe), media_(media), video_(nullptr)
          {
          }

          ~VividCameraData()
          {
                delete video_;
          }

          int init();
          void bufferReady(FrameBuffer *buffer);

          MediaDevice *media_;
          V4L2VideoDevice *video_;
          Stream stream_;
   };

This example pipeline handler handles a single video device and supports a
single stream, represented by the ``VividCameraData`` class members. More complex
pipeline handlers might register cameras composed of several video devices and
sub-devices, or multiple streams per camera that represent the several
components of the image capture pipeline. You should represent all these
components in the ``CameraData`` derived class.

The camera instance specific data can be initialized with an optional ``init()``
method. The base ``CameraData`` class doesn’t define an ``init()`` function to
overload, it’s then up to pipeline handlers to define how they initialize the
camera and camera data. This method is one of the more device-specific methods
for a pipeline handler, and defines the context of the camera, and how libcamera
should communicate with the camera and store the data it generates. For real
hardware, this includes tasks such as opening the ISP, or creating a sensor
device.

For this example, create an ``init`` method after the ``VividCameraData`` class
that creates a new V4L2 video device by matching the media entity name of a
device using the `MediaDevice::getEntityByName
<http://libcamera.org/api-html/classlibcamera_1_1MediaDevice.html#ad5d9279329ef4987ceece2694b33e230>`_
helper.

.. code-block:: cpp

   int VividCameraData::init()
   {
          video_ = new V4L2VideoDevice(media_->getEntityByName("vivid-000-vid-cap"));
          if (video_->open())
                return -ENODEV;

          return 0;
   }

Return to the ``match`` method, and remove ``LOG(VIVID, Debug) << "Obtained
Vivid Device";`` and ``return false; // Prevent infinite loops for now``,
replacing it with the following code.

After a successful device match, the code below creates a new instance of the
device-specific ``CameraData`` class, using a unique pointer to manage the
lifetime of the instance.

If the camera data initialization fails, return ``false`` to indicate the
failure to the ``match()`` method and prevent retiring of the pipeline handler.

.. code-block:: cpp

   std::unique_ptr<VividCameraData> data = std::make_unique<VividCameraData>(this, media);

   if (data->init())
           return false;

Once the camera data has been initialized, the Camera device instances and the
associated streams have to be registered. Create a set of streams for the
camera, which for this device is only one. You create a camera using the static
`Camera::create
<http://libcamera.org/api-html/classlibcamera_1_1Camera.html#a453740e0d2a2f495048ae307a85a2574>`_
method, passing the pipeline handler, the name of the camera, and the streams
available. Then register the camera and its data with the camera manager using
`registerCamera
<http://libcamera.org/api-html/classlibcamera_1_1PipelineHandler.html#adf02a7f1bbd87aca73c0e8d8e0e6c98b>`_.
At the end of the method, return ``true`` to express that a camera was created
successfully.

Add the following below the code added above:

.. code-block:: cpp

   std::set<Stream *> streams{ &data->stream_ };
   std::shared_ptr<Camera> camera = Camera::create(this, data->video_->deviceName(), streams);
   registerCamera(std::move(camera), std::move(data));

   return true;

Add a private ``cameraData`` helper to the ``PipelineHandlerVivid`` class which
obtains the camera data, and does the necessary casting to convert it to the
pipeline-specific ``VividCameraData``. This simplifies the process of obtaining
the custom camera data, which you need throughout the code for the pipeline
handler.

.. code-block:: cpp

   private:
       VividCameraData *cameraData(const Camera *camera)
       {
               return static_cast<VividCameraData *>(
                        PipelineHandler::cameraData(camera));
       }

At this point, you need to add the following new includes to provide the Camera
interface, and device interaction interfaces.

.. code-block:: cpp

   #include <libcamera/camera.h>
   #include "libcamera/internal/media_device.h"
   #include "libcamera/internal/v4l2_videodevice.h"

Registering controls and properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The libcamera `controls framework
<http://libcamera.org/api-html/controls_8h.html>`_ allows application to
configure the streams capture parameters on a per-frame basis and is also used
to advertise to application the ``Camera`` device immutable properties. The
currently growing list of libcamera defined controls and camera properties is
available in the `control_ids.yaml
<http://libcamera.org/api-html/control__ids_8h.html>`_ and in the
`properties_ids.yaml <http://libcamera.org/api-html/property__ids_8h.html>`_
files.

Pipeline handlers can optionally register the list of controls an application
can set as well as a list of immutable camera properties. Being both
Camera-specific values, they are represented in the ``CameraData`` base class,
which provides two members for this purpose: the `CameraData::controlInfo_
<http://libcamera.org/api-html/classlibcamera_1_1CameraData.html#ab9fecd05c655df6084a2233872144a52>`_
and the `CameraData::properties_
<http://libcamera.org/api-html/classlibcamera_1_1CameraData.html#a84002c29f45bd35566c172bb65e7ec0b>`_
fields.

The ``controlInfo_`` field represents a map of ``ControlId`` instances
associated with the limits of valid values supported for the control. More
information can be found in the `ControlnfoMap
<http://libcamera.org/api-html/classlibcamera_1_1ControlInfoMap.html>`_ class
documentation.

Pipeline handlers register controls to expose to applications the video devices
tunable parameters controlled using v4l2-ctrls framework, and parameters of
the image processing algorithms. The example pipeline handler only expose
trivial controls of the video device, by registering a ``ControlId`` instance
with associated values for each supported V4L2 control.

Complete the initialization of the ``VividCameraData`` class by adding the
following code to the ``VividCameraData::init()`` method:

.. code-block:: cpp

   /* Initialise the supported controls. */
   const ControlInfoMap &controls = video_->controls();
   ControlInfoMap::Map ctrls;

   for (const auto &ctrl : controls) {
           const ControlId *id;
           ControlInfo info;

           switch (ctrl.first->id()) {
           case V4L2_CID_BRIGHTNESS:
                   id = &controls::Brightness;
                   info = ControlInfo{ { -1.0f }, { 1.0f }, { 0.0f } };
                   break;
           case V4L2_CID_CONTRAST:
                   id = &controls::Contrast;
                   info = ControlInfo{ { 0.0f }, { 2.0f }, { 1.0f } };
                   break;
           case V4L2_CID_SATURATION:
                   id = &controls::Saturation;
                   info = ControlInfo{ { 0.0f }, { 2.0f }, { 1.0f } };
                   break;
           default:
                   continue;
           }

           ctrls.emplace(id, info);
   }

   controlInfo_ = std::move(ctrls);

The ``properties_`` field is instead a list of ``ControlId`` instances
associated with immutable values, which represent static characteristics the
applications can use to identify camera devices in the system. Properties can be
registered inspecting the values of V4L2 controls from the video devices and
camera sensor (in example to retrieve the position and orientation of a camera)
or to express other immutable characteristics. The example pipeline handler does
not register any property, but examples are available in the libcamera code
base.

At this point you need to add the following includes to the top of the file for
controls handling:

.. code-block:: cpp

   #include <libcamera/controls.h>
   #include <libcamera/control_ids.h>

Generating a default configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once ``Camera`` devices and the associated ``Stream`` have been registered, an
application can now proceed to configure the system to prepare it for a frame
capture session.

Applications specify the requested system configuration by assigning to each
stream they want to enable a ``StreamConfiguration`` instance which expresses
the desired size and image format. The stream configurations are grouped in a
``CameraConfiguration`` the pipeline handler inspects and validate to adjust it
to a supported configuration by, in example, adjusting the formats or the image
size alignments. The pipeline handler receives a valid camera configuration and
use the image stream configurations to apply settings to the hardware devices.

Create a `CameraConfiguration
<http://libcamera.org/api-html/classlibcamera_1_1CameraConfiguration.html>`_
derived class for the camera device and its empty constructor before the
``PipelineHandlerVivid`` class.

The ``CameraConfiguration`` derived class overrides the base class
``validate()`` function, where the stream configuration inspection and
adjustment happens.

.. code-block:: cpp

    class VividCameraConfiguration : public CameraConfiguration
    {
    public:
           VividCameraConfiguration();

           Status validate() override;
    };

    VividCameraConfiguration::VividCameraConfiguration()
           : CameraConfiguration()
    {
    }

Applications generate a ``CameraConfiguration`` instance by calling
the `Camera::generateConfiguration()
<http://libcamera.org/api-html/classlibcamera_1_1Camera.html#a25c80eb7fc9b1cf32692ce0c7f09991d>`_
function, which is realized by the pipeline handler implementation of the
overridden `generateConfiguration
<http://libcamera.org/api-html/classlibcamera_1_1PipelineHandler.html#a7932e87735695500ce1f8c7ae449b65b>`_
method.

Configurations are generated by receiving a list of ``StreamRoles`` instances,
which libcamera uses to define the predefined ways an application intends to use
a camera (`You can read the full list in the API documentation
<http://libcamera.org/api-html/stream_8h.html#file_a295d1f5e7828d95c0b0aabc0a8baac03>`_).
These are optional hints on how an application intends to use a stream, and a
pipeline handler should return ideal configuration for each role an application
requests.

In the pipeline handler ``generateConfiguration`` implementation, remove the
``return nullptr;``, create a new instance of the ``CameraConfiguration``
derived class, and assign it to a base class pointer.

.. code-block:: cpp

   CameraConfiguration *config = new VividCameraConfiguration();
   VividCameraData *data = cameraData(camera);

A ``CameraConfiguration`` is specific to each pipeline, so you can only create
it from the pipeline handler code path. Application can generate empty
configuration and add desired stream configuration manually. To allow for
this, add the following beneath the code above to return the newly constructed
empty configuration in case the application does not pass any ``StreamRole``.

.. code-block:: cpp

   if (roles.empty())
           return config;

A production pipeline handler should generate the ``StreamConfiguration`` for
all the appropriate stream roles a camera device supports. For this simpler
example (with only one stream), the pipeline handler always returns the same
configuration. How it does this is reproduced below, but we recommend you take a
look at full-featured pipeline handlers in the libcamera code base for a
realistic example.

.. TODO: Add link

To generate a ``StreamConfiguration``, you need a list of pixel formats and
frame sizes supported by the device. You can fetch a map of the
``V4LPixelFormat`` and ``SizeRange`` supported by the device, but the pipeline
handler needs to convert this to a ``libcamera::PixelFormat`` type to pass to
applications. You can do this using ``std::transform`` to convert the formats
and populate a new ``PixelFormat`` map as shown below. Add the following beneath
the code from above.

.. code-block:: cpp

   std::map<V4L2PixelFormat, std::vector<SizeRange>> v4l2Formats =
   data->video_->formats();
   std::map<PixelFormat, std::vector<SizeRange>> deviceFormats;
   std::transform(v4l2Formats.begin(), v4l2Formats.end(),
          std::inserter(deviceFormats, deviceFormats.begin()),
          [&](const decltype(v4l2Formats)::value_type &format) {
              return decltype(deviceFormats)::value_type{
                  format.first.toPixelFormat(),
                  format.second
              };
          });

The `StreamFormats
<http://libcamera.org/api-html/classlibcamera_1_1StreamFormats.html>`_ class
holds information about the pixel formats and frame sizes a stream supports. The
class groups size information by the pixel format, which can produce it.

The code below uses the ``StreamFormats`` class to represent all the pixel
formats a stream supports, associated with a list of frame sizes. It then
associates the supported configurations with the ``StreamConfiguration`` class
instance to model the information an application can use to configure a single
stream.

Add the following below the code from above:

.. code-block:: cpp

   StreamFormats formats(deviceFormats);
   StreamConfiguration cfg(formats);

Create the default values for pixel formats, sizes, and buffer count returned by
the configuration.

Add the following below the code from above:

.. code-block:: cpp

   cfg.pixelFormat = formats::BGR888;
   cfg.size = { 1280, 720 };
   cfg.bufferCount = 4;

Add each ``StreamConfiguration`` you generate to the ``CameraConfiguration``,
and finally validate it before returning it to the application.

Add the following below the code from above:

.. code-block:: cpp

   config->addConfiguration(cfg);

   config->validate();

   return config;

To validate a camera configuration, a pipeline handler must implement the
`validate
<http://libcamera.org/api-html/classlibcamera_1_1CameraConfiguration.html#a29f8f263384c6149775b6011c7397093>`_
method that inspects all the stream configuration associated to it, make
adjustments to make the configuration valid, and returns the validation status.
If changes are made, it marks the configuration as ``Adjusted``. If the
requested configuration is not supported and cannot be adjusted it shall be
refused and marked as ``Invalid``.

The validation phase makes sure all the platform-specific constraints are
respected by the requested configuration. The most trivial examples being making
sure the requested image formats are supported and the image alignment
constraints respected. The pipeline handler implementation of ``validate()``
shall inspect all the received configurations and never assume they are correct,
as applications are free to change the requested stream parameters after the
configuration has been generated.

Again, this example pipeline handler is simpler, look at the more complex
implementations for a realistic example.

.. TODO: Add link

Add the following code above ``PipelineHandlerVivid::configure``:

.. code-block:: cpp

   CameraConfiguration::Status VividCameraConfiguration::validate()
   {
           Status status = Valid;

           if (config_.empty())
                  return Invalid;

           if (config_.size() > 1) {
                  config_.resize(1);
                  status = Adjusted;
           }

           StreamConfiguration &cfg = config_[0];

           const std::vector<libcamera::PixelFormat> formats = cfg.formats().pixelformats();
           if (std::find(formats.begin(), formats.end(), cfg.pixelFormat) == formats.end()) {
                  cfg.pixelFormat = cfg.formats().pixelformats()[0];
                  LOG(VIVID, Debug) << "Adjusting format to " << cfg.pixelFormat.toString();
                  status = Adjusted;
           }

           cfg.bufferCount = 4;

           return status;
   }

To handle ``PixelFormat``, add ``#include <libcamera/formats.h>`` to the
include section, rebuild the codebase, and use:

.. code-block:: shell

   LIBCAMERA_LOG_LEVELS=Pipeline,VIVID:0 ./build/src/cam/cam -c vivid -I

To test the configuration is generated.

You should see the following output:

.. code-block:: shell

    Using camera vivid
    0: 1280x720-BGR888
    * Pixelformat: NV21 (320x180)-(3840x2160)/(+0,+0)
    - 320x180
    - 640x360
    - 640x480
    - 1280x720
    - 1920x1080
    - 3840x2160
    * Pixelformat: NV12 (320x180)-(3840x2160)/(+0,+0)
    - 320x180
    - 640x360
    - 640x480
    - 1280x720
    - 1920x1080
    - 3840x2160
    * Pixelformat: BGRA8888 (320x180)-(3840x2160)/(+0,+0)
    - 320x180
    - 640x360
    - 640x480
    - 1280x720
    - 1920x1080
    - 3840x2160
    * Pixelformat: RGBA8888 (320x180)-(3840x2160)/(+0,+0)
    - 320x180
    - 640x360
    - 640x480
    - 1280x720
    - 1920x1080
    - 3840x2160

Configuring a device
~~~~~~~~~~~~~~~~~~~~

With the configuration generated, and optionally modified and validated, a
pipeline handler needs a method that allows an application to apply a
configuration to the hardware devices.

The `PipelineHandler::configure()
<http://libcamera.org/api-html/classlibcamera_1_1PipelineHandler.html#a930f2a9cdfb51dfb4b9ca3824e84fc29>`_
method receives a valid `CameraConfiguration
<http://libcamera.org/api-html/classlibcamera_1_1CameraConfiguration.html>`_ and
applies the settings to hardware devices, using its content to prepare a device
for a streaming session.

Replace the contents of the ``PipelineHandlerVivid::configure`` method with the
following that obtains the camera data and stream configuration. This pipeline
handler supports only a single stream, so it directly obtains the first
``StreamConfiguration`` from the camera configuration. A pipeline handler with
multiple streams should configure the system inspecting each of them.

.. code-block:: cpp

   VividCameraData *data = cameraData(camera);
   StreamConfiguration &cfg = config->at(0);
   int ret;

The Vivid capture device is a V4L2 video device, so create a `V4L2DeviceFormat
<http://libcamera.org/api-html/classlibcamera_1_1V4L2DeviceFormat.html>`_ with
the fourcc and size attributes to apply directly to the capture device node. The
fourcc attribute is a `V4L2PixelFormat
<http://libcamera.org/api-html/classlibcamera_1_1V4L2PixelFormat.html>`_ and
differs from the ``libcamera::PixelFormat``.  Converting the format requires
knowledge of the plane configuration for multiplanar formats, so you must
explicitly convert it using the helpers provided by the ``V4LVideoDevice``, in
this case ``toV4L2PixelFormat``.

Add the following code beneath the code from above:

.. code-block:: cpp

   V4L2DeviceFormat format = {};
   format.fourcc = data->video_->toV4L2PixelFormat(cfg.pixelFormat);
   format.size = cfg.size;

Set the video device format defined above using the `setFormat
<http://libcamera.org/api-html/classlibcamera_1_1V4L2VideoDevice.html#ad67b47dd9327ce5df43350b80c083cca>`_
helper method. You should check if the kernel driver has adjusted the format, as
this shows the pipeline handler has failed to handle the validation stages
correctly, and the configure operation shall also fail.

Add the following code beneath the code from above:

.. code-block:: cpp

   ret = data->video_->setFormat(&format);
   if (ret)
          return ret;

   if (format.size != cfg.size ||
          format.fourcc != data->video_->toV4L2PixelFormat(cfg.pixelFormat))
          return -EINVAL;

Finally, store and set stream-specific data reflecting the state of the stream.
Associate the configuration with the stream by using the `setStream
<http://libcamera.org/api-html/structlibcamera_1_1StreamConfiguration.html#a74a0eb44dad1b00112c7c0443ae54a12>`_
method, and you can also set the values of individual stream configuration
members.

.. NOTE: the cfg.setStream() call here associates the stream to the
   StreamConfiguration however that should quite likely be done as part of
   the validation process. TBD

Add the following code beneath the code from above:

.. code-block:: cpp

   cfg.setStream(&data->stream_);
   cfg.stride = format.planes[0].bpl;

   return 0;

.. NOTE: stride SHALL be assigned in validate

Initializing device controls
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Pipeline handlers can optionally initialize the video devices and camera sensor
controls at system configuration time, to make sure to make sure they are
defaulted to sane values. Handling of device controls is again performed using
the libcamera `controls framework
<http://libcamera.org/api-html/controls_8h.html>`_.

This section is particularly vivid specific as it sets the initial values of
controls to match `the controls that vivid defines
<https://www.kernel.org/doc/html/latest/admin-guide/media/vivid.html#controls>`_.
You won’t need any of the code below for your pipeline handler, but it’s
included as an example of how to implement what your pipeline handler might
need.

Create a list of controls with the `ControlList
<http://libcamera.org/api-html/classlibcamera_1_1ControlList.html>`_ class, and
set them using the `set
<http://libcamera.org/api-html/classlibcamera_1_1ControlList.html#a74a1a29abff5243e6e37ace8e24eb4ba>`_
method.

Create defines beneath the current includes for convenience:

.. code-block:: cpp

   #define VIVID_CID_VIVID_BASE            (0x00f00000 | 0xf000)
   #define VIVID_CID_VIVID_CLASS           (0x00f00000 | 1)
   #define VIVID_CID_TEST_PATTERN          (VIVID_CID_VIVID_BASE  + 0)
   #define VIVID_CID_OSD_TEXT_MODE         (VIVID_CID_VIVID_BASE  + 1)
   #define VIVID_CID_HOR_MOVEMENT          (VIVID_CID_VIVID_BASE  + 2)
   #define VIVID_CID_VERT_MOVEMENT         (VIVID_CID_VIVID_BASE  + 3)
   #define VIVID_CID_SHOW_BORDER           (VIVID_CID_VIVID_BASE  + 4)
   #define VIVID_CID_SHOW_SQUARE           (VIVID_CID_VIVID_BASE  + 5)
   #define VIVID_CID_INSERT_SAV            (VIVID_CID_VIVID_BASE  + 6)
   #define VIVID_CID_INSERT_EAV            (VIVID_CID_VIVID_BASE  + 7)
   #define VIVID_CID_VBI_CAP_INTERLACED    (VIVID_CID_VIVID_BASE  + 8)

In the ``configure`` method, add the below above the
``cfg.setStream(&data->stream_);`` line:

.. code-block:: cpp

   ControlList controls(data->video_->controls());
   controls.set(VIVID_CID_TEST_PATTERN, 0);
   controls.set(VIVID_CID_OSD_TEXT_MODE, 0);

   controls.set(V4L2_CID_BRIGHTNESS, 128);
   controls.set(V4L2_CID_CONTRAST, 128);
   controls.set(V4L2_CID_SATURATION, 128);

   controls.set(VIVID_CID_HOR_MOVEMENT, 5);

   ret = data->video_->setControls(&controls);
   if (ret) {
          LOG(VIVID, Error) << "Failed to set controls: " << ret;
          return ret < 0 ? ret : -EINVAL;
   }

These controls configure VIVID to use a default test pattern, and enable all
on-screen display text, while configuring sensible brightness, contrast and
saturation values. Use the ``controls.set`` method to set individual controls.

Buffer handling and stream control
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once the system has been configured with the requested parameters, it is now
possible for applications to start capturing frames from the ``Camera`` device.

Libcamera implements a per-frame request capture model, realized by queueing
``Request`` instances to a ``Camera`` object. Before applications can start
submitting capture requests the capture pipeline needs to be prepared to deliver
frames as soon as they are requested. Memory should be initialized and made
available to the devices which have then to be started to be ready to produce
images. At the end of a capture session the ``Camera`` device needs to be
stopped, to gracefully clean up any allocated memory and stop the hardware
devices. Pipeline handlers implement two methods for these purposes, the
``start()`` and ``stop()`` methods.

The memory initialization phase the happens at ``start()`` time serves to
configure video devices to be able to use memory buffers exported as dma-buf
file descriptors. From the pipeline handler perspective the video devices that
provide application facing streams always act as memory importers which use,
in V4L2 terminology, buffer of V4L2_MEMORY_DMABUF memory type.

Libcamera also provides an API to allocate and export memory to applications
realized through the `exportFrameBuffers
<http://libcamera.org/api-html/classlibcamera_1_1PipelineHandler.html#a6312a69da7129c2ed41f9d9f790adf7c>`_
function and the `FrameBufferAllocator
<http://libcamera.org/api-html/classlibcamera_1_1FrameBufferAllocator.html>`_
class.This API will be presented later.

Please refer to the V4L2VideoDevice API documentation, specifically the
`allocateBuffers
<http://libcamera.org/api-html/classlibcamera_1_1V4L2VideoDevice.html#a3a1a77e5e6c220ea7878e89485864a1c>`_
, `importBuffers
<http://libcamera.org/api-html/classlibcamera_1_1V4L2VideoDevice.html#a154f5283d16ebd5e15d63e212745cb64>_`
and `exportBuffers
<http://libcamera.org/api-html/classlibcamera_1_1V4L2VideoDevice.html#ae9c0b0a68f350725b63b73a6da5a2ecd>_`
functions for a detailed description of the video device memory management.

Video memory buffers are represented in libcamera by the `FrameBuffer
<http://libcamera.org/api-html/classlibcamera_1_1FrameBuffer.html>`_ class.
A ``FrameBuffer`` instance has to be associated to each ``Stream`` which is part
of a capture ``Request``. Pipeline handlers should prepare the capture devices
by importing the dma-buf file descriptors it needs to operate on. This operation
is performed by using the ``V4L2VideoDevice`` API, which provides an
``importBuffers()`` function that prepares the video device.

Implement the pipeline handler ``start()`` function by replacing the stub
version with the following code:

.. code-block:: c++

   VividCameraData *data = cameraData(camera);
   unsigned int count = data->stream_.configuration().bufferCount;

   int ret = data->video_->importBuffers(count);
   if (ret < 0)
         return ret;

   return 0;

During the startup phase pipeline handlers shall setup up any internal buffer
pool required to transfer data between different components of the image capture
pipeline, in example, between the CSI-2 receiver and the ISP input. The example
pipeline does not require any internal pool, but examples are available in more
complex pipeline handlers in the libcamera code base.

Applications might want to use memory allocated in the video devices
instead of allocating it from other parts of the system. Libcamera
provides an abstraction to ease this task in the `FrameBufferAllocator
<http://libcamera.org/api-html/classlibcamera_1_1FrameBufferAllocator.html>`_
class. The ``FrameBufferAllocators`` reserves memory for a ``Stream`` in the
video device and exports it as dma-buf file descriptors. From
this point on, the allocated ``FrameBuffer`` are associated to ``Stream``
instances in a ``Request`` and then imported by the pipeline hander exactly
as they where allocated from elsewhere.

Pipeline handlers supports the ``FrameBufferAllocator`` operations by
implementing the `exportFrameBuffers
<http://libcamera.org/api-html/classlibcamera_1_1PipelineHandler.html#a6312a69da7129c2ed41f9d9f790adf7c>`_
function, that allocates memory in the video device associated with a stream and
exports it.

Implement the ``exportFrameBuffers`` stub method with the following code:

.. code-block:: cpp

   unsigned int count = stream->configuration().bufferCount;
   VividCameraData *data = cameraData(camera);

   return data->video_->exportBuffers(count, buffers);

Once memory has been properly setup, the video devices can be started to prepare
for capture operations. Complete the ``start`` method implementation with
the following code:

.. code-block:: cpp

   ret = data->video_->streamOn();
   if (ret < 0) {
          data->video_->releaseBuffers();
          return ret;
   }

   return 0;

The method starts the video device associated with the stream with the `streamOn
<http://libcamera.org/api-html/classlibcamera_1_1V4L2VideoDevice.html#a588a5dc9d6f4c54c61136ac43ff9a8cc>`_
method. If the call fails, the error value is propagated to the caller
and the `releaseBuffers
<http://libcamera.org/api-html/classlibcamera_1_1V4L2VideoDevice.html#a191619c152f764e03bc461611f3fcd35>`_
method releases any buffers to leave the device in a consistent state. If your
pipeline handler uses any image processing algorithms, you should also stop
them.

Add the following to the ``stop`` method, which stops the stream
(`streamOff <http://libcamera.org/api-html/classlibcamera_1_1V4L2VideoDevice.html#a61998710615bdf7aa25a046c8565ed66>`_)
and releases the buffers (``releaseBuffers``).

.. code-block:: cpp

   VividCameraData *data = cameraData(camera);
   data->video_->streamOff();
   data->video_->releaseBuffers();

Queuing requests between applications and hardware
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

libcamera implements a streaming model based on capture requests queued by
application to the ``Camera`` device. Each requests contains at least one
``Stream`` instance with associated a ``FrameBuffer`` object.

When an application sends a capture request, the pipeline handler
identifies which video devices have to be provided with buffers to generate a
frame from the enabled streams.

This example pipeline handler identifies the buffer (`findBuffer
<http://libcamera.org/api-html/classlibcamera_1_1Request.html#ac66050aeb9b92c64218945158559c4d4>`_)
from the only supported stream and queues it to the capture device (`queueBuffer
<http://libcamera.org/api-html/classlibcamera_1_1V4L2VideoDevice.html#a594cd594686a8c1cf9ae8dba0b2a8a75>`_).

Replace the contents of ``queueRequestDevice`` with the following:

.. code-block:: cpp

   VividCameraData *data = cameraData(camera);
   FrameBuffer *buffer = request->findBuffer(&data->stream_);
   if (!buffer) {
          LOG(VIVID, Error)
                  << "Attempt to queue request with invalid stream";

          return -ENOENT;
    }

   int ret = data->video_->queueBuffer(buffer);
   if (ret < 0)
          return ret;

   return 0;

Processing controls
~~~~~~~~~~~~~~~~~~~

Capture requests not only contains streams and memory buffer, but could
optionally contain a list of controls the application set to modify the
streaming parameters.

Application can set controls registered by the pipeline handler in the
initialization phase, as explained in the 'Registering controls and properties'
section.

Create the ``processControls`` method above the ``queueRequestDevice`` method.
The method loops through the control list received with a request, and inspect
the received values to convert between the libcamera control range definitions
and their corresponding values on the device.

.. code-block:: cpp

   int PipelineHandlerVivid::processControls(VividCameraData *data, Request *request)
   {
          ControlList controls(data->video_->controls());

          for (auto it : request->controls()) {
                 unsigned int id = it.first;
                 unsigned int offset;
                 uint32_t cid;

                 if (id == controls::Brightness) {
                        cid = V4L2_CID_BRIGHTNESS;
                        offset = 128;
                 } else if (id == controls::Contrast) {
                        cid = V4L2_CID_CONTRAST;
                        offset = 0;
                 } else if (id == controls::Saturation) {
                        cid = V4L2_CID_SATURATION;
                        offset = 0;
                 } else {
                        continue;
                 }

                 int32_t value = lroundf(it.second.get<float>() * 128 + offset);
                 controls.set(cid, utils::clamp(value, 0, 255));
          }

          for (const auto &ctrl : controls)
                 LOG(VIVID, Debug)
                        << "Setting control " << utils::hex(ctrl.first)
                        << " to " << ctrl.second.toString();

          int ret = data->video_->setControls(&controls);
          if (ret) {
                 LOG(VIVID, Error) << "Failed to set controls: " << ret;
                 return ret < 0 ? ret : -EINVAL;
          }

          return ret;
   }

Declare the function prototype for the ``processControls`` method within the
private ``PipelineHandlerVivid`` class members, as it is only used internally as
a helper when processing Requests.

.. code-block:: cpp

   private:
        int processControls(VividCameraData *data, Request *request);

A pipeline handler is responsible for applying controls provided in a Request to
the relevant hardware devices. This could be directly on the capture device, or
where appropriate by setting controls on V4L2Subdevices directly. Each pipeline
handler is responsible for understanding the correct procedure for applying
controls to the device they support.

This example pipeline handler applies controls during the `queueRequestDevice
<http://libcamera.org/api-html/classlibcamera_1_1PipelineHandler.html#a106914cca210640c9da9ee1f0419e83c>`_
method for each request, and applies them to the capture device through the
capture node.

In the ``queueRequestDevice`` method, replace the following:

.. code-block:: cpp

   int ret = data->video_->queueBuffer(buffer);
   if (ret < 0)
        return ret;

With the following code:

.. code-block:: cpp

   int ret = processControls(data, request);
   if (ret < 0)
        return ret;

   ret = data->video_->queueBuffer(buffer);
   if (ret < 0)
        return ret;

Add the following inclusion directive to support the control value translate
operations:

.. code-block:: cpp

   #include <math.h>

Frame completion and event handling
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Libcamera implements a signals and slots mechanism (`similar to Qt
<https://doc.qt.io/qt-5/signalsandslots.html>`_) to connect event sources with
callbacks to handle them.

As a general summary, a ``Slot`` can be connected to a ``Signal``, which when
emitted triggers the execution of the connected slots.  A detailed description
of the libcamera implementation is available in the `Signal and Slot classes
documentation
<http://libcamera.org/api-html/classlibcamera_1_1Signal.html#details>`_.

In order to notify applications about the availability of new frames and data,
the ``Camera`` device exposes two ``Signals`` to which applications can connect
to be notified of frame completion events. The ``bufferComplete`` signal serves
to report to applications the completion of a single ``Stream`` part of a
``Requests``, while the ``requestComplete`` signal notifies the completion of
all the ``Streams`` and data submitted as part of a request. This mechanism
allows implementation of partial request completion, that allows an application
to inspect completed buffers associated with the single streams without waiting
for all of them to be ready.

The ``bufferComplete`` and ``requestComplete`` signals are emitted by the
``Camera`` device upon notifications received from the pipeline handler, which
tracks the buffers and request completion statuses.

The single buffer completion notification is implemented by pipeline handlers by
`connecting
<http://libcamera.org/api-html/classlibcamera_1_1Signal.html#aa04db72d5b3091ffbb4920565aeed382>`_
the ``bufferReady`` signal of the capture devices they have queued buffers to,
to a member function slot that handles processing of the completed frames. When
a buffer is ready, the pipeline handler must propagate the completion of that
buffer to the Camera by using the PipelineHandler base class ``completeBuffer``
function. When all the buffers part of a ``Request`` have been completed, the
pipeline handler again must notify it to the ``Camera`` using the
PipelineHandler base class ``completeRequest`` function. The PipelineHandler
class implementation makes sure the request completion notifications are
delivered to applications in the same order as they have been submitted.

In this example, when a buffer completes, the event handler calls the buffer
completion slot of the pipeline handler which, because the device has a single
stream, immediately completes the request.

Returning to the ``int VividCameraData::init()`` method, add the following above
the closing ``return 0;`` to connects the pipeline handler ``bufferReady``
method to the V4L2 device buffer signal.

.. code-block:: cpp

   video_->bufferReady.connect(this, &VividCameraData::bufferReady);

Create the matching ``VividCameraData::bufferReady`` method above the
``REGISTER_PIPELINE_HANDLER(PipelineHandlerVivid);`` line that takes the frame
buffer passed to it as a parameter.

The ``bufferReady`` method obtains the request from the buffer using the
``request`` method, and notifies the ``Camera`` that the buffer and
request are completed. In this simpler pipeline handler, there is only one
buffer, so it completes the request immediately. You can find a more complex
example of event handling with supporting multiple streams in the libcamera
code-base.

.. TODO: Add link

.. code-block:: cpp

   void VividCameraData::bufferReady(FrameBuffer *buffer)
   {
          Request *request = buffer->request();

          pipe_->completeBuffer(camera_, request, buffer);
          pipe_->completeRequest(camera_, request);
   }

Testing a pipeline handler
~~~~~~~~~~~~~~~~~~~~~~~~~~

Once you've built the pipeline handler, rebuild the code base, and you can use
the following command:

.. code-block:: shell

   LIBCAMERA_LOG_LEVELS=Pipeline,VIVID:0 ./build/src/cam/cam -c vivid -I -C

To test that the pipeline handler can detect a device, and capture input.

Running the command above outputs (a lot of) information about pixel formats,
and then starts capturing frame data.



.. TODO: LIBCAMERA_LOG_LEVELS=Pipeline,VIVID:0 sudo ./build/src/qcam/qcam -c vivid

