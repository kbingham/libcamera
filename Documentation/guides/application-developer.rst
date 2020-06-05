.. SPDX-License-Identifier: CC-BY-SA-4.0

Using libcamera in a C++ application
====================================

This tutorial shows how to create a C++ application that uses libcamera
to interface with a camera on a system, capture frames from it for 3
seconds, and write metadata about the frames to standard out.

.. TODO: Check how much of the example code runs before camera start etc?

Application skeleton
--------------------

Most of the code in this tutorial runs in the ``int main()`` function
with a separate global function to handle events. The two functions need
to share data, which are stored in global variables for simplicity. A
production-ready application would organize the various objects created
in classes, and the event handler would be a class member function to
provide context data without requiring global variables.

Use the following code snippets as the initial application skeleton.
It already lists all the necessary includes directives and instructs the
compiler to use the libcamera namespace, which gives access to the libcamera
defined names and types without the need of prefixing them.

.. code:: cpp

   #include <iomanip>
   #include <iostream>
   #include <memory>

   #include <libcamera/libcamera.h>

   using namespace libcamera;

   int main()
   {
       // Code to follow

       return 0;
   }

Camera Manager
--------------

Every libcamera-based application needs an instance of a `CameraManager
<http://libcamera.org/api-html/classlibcamera_1_1CameraManager.html>`_ that runs
for the life of the application. When the Camera Manager starts, it enumerates
all the cameras detected in the system. Behind the scenes, libcamera abstracts
and manages the complex pipelines that kernel drivers expose through the `Linux
Media Controller
<https://www.kernel.org/doc/html/latest/media/uapi/mediactl/media-controller-intro.html>`_
and `Video for Linux (V4L2) <https://www.linuxtv.org/docs.php>`_ APIs, meaning
that an application doesn’t need to handle device or driver specific details.

Create a Camera Manager instance at the beginning of the main function, and then
start it. An application should only create a single Camera Manager instance.

.. code:: cpp

   CameraManager *cm = new CameraManager();
   cm->start();

During the application initialization, the Camera Manager is started to
enumerates all the supported devices and creates cameras the application can
interact with.

Before the ``int main()`` function, create a global shared pointer
variable for the camera:

.. code:: cpp

   std::shared_ptr<Camera> camera;

   int main()
   {
       // Code to follow
   }

Add the code below that lists all available cameras, and for this
example, writes their ids to standard output:

.. code:: cpp

   for (auto const &camera : cm->cameras())
       std::cout << camera->id() << std::endl;

Printing the camera id lists the machine-readable identifiers, so for example,
the output on a Linux machine with a connected USB webcam
is ``\_SB_.PCI0.XHC_.RHUB.HS08-8:1.0-5986:2115``.

Create and acquire a camera
---------------------------

What libcamera considers a camera
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The libcamera library considers any unique source of video frames, which usually
correspond to a camera sensors, as a single camera devices. Camera devices
expose streams, which are obtained by processing data from the single image
source and all share some basic properties such as the frame duration and the
images exposure time, as they only depend by the image source configuration.

Applications select one or multiple Camera devices they wish to operate on, and
require frames from at least one of their Streams.

This example application uses a single camera (the first enumerated one) that
the Camera Manager reports as available to applications.

Camera devices are stored by the CameraManager in a list accessible by index, or
can be retrieved by name through the ``CameraManager::get()`` function. The
code below retrieves the name of the first available camera and gets the camera
by name from the Camera Manager.

.. code:: cpp

   std::string cameraId = cm->cameras()[0]->id();
   camera = cm->get(cameraId);

   /*
    * Note that is equivalent to:
    * camera = cm->cameras()[0];
    */

Once a camera has been selected an application needs to acquire an exclusive
lock to it so no other application can use it.

.. code:: cpp

   camera->acquire();

Configure the camera
--------------------

Before the application can do anything with the camera, it needs to configure
the image format and sizes of the streams it wants to capture frames from.

Streams configurations are represented by instances of the
``StreamConfiguration`` class, which are grouped together in a
``CameraConfiguration`` object. Before an application can start setting its
desired configuration, a ``CameraConfiguration`` instance needs to be generated
from the ``Camera`` device using the ``Camera::generateConfiguration()``
function.

The libcamera library uses the ``StreamRole`` enumeration to define predefined
ways an application intends to use a camera. The
``Camera::generateConfiguration()`` function accepts a list of desired roles and
generates a ``CameraConfiguration`` with the best stream parameters
configuration for each of the requested roles.  If the camera can handle the
requested roles, it returns an initialized ``CameraConfiguration`` and if it
can't, a null pointer.

It is possible for applications to generate an empty ``CameraConfiguration``
instance by not providing any role. The desired configuration will have to be
filled-in manually and manually validated.

In the example application, create a new configuration variable and use the
``Camera::generateConfiguration`` function to produce a ``CameraConfiguration``
for the single ``StreamRole::Viewfinder`` role.

.. code:: cpp

   std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::Viewfinder } );

The generated ``CameraConfiguration`` has a ``StreamConfiguration`` instance for
each ``StreamRole`` the application requested. Each of these has a default size
and format that the camera assigned, and a list of supported pixel formats and
sizes.

The code below accesses the first and only ``StreamConfiguration`` item in the
``CameraConfiguration`` and outputs its parameters to standard output.

.. code:: cpp

   StreamConfiguration &streamConfig = config->at(0);
   std::cout << "Default viewfinder configuration is: " << streamConfig.toString() << std::endl;

This outputs something like
``Default viewfinder configuration is: 1280x720-MJPEG``

Change and validate the configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With an initialized ``CameraConfiguration``, an application can make changes
to the parameters it contains, for example, to change the width and
height, use the following code:

.. code:: cpp

   streamConfig.size.width = 640;
   streamConfig.size.height = 480;

If an application changes any parameters, it shall validate the configuration
before applying it to the camera using the ``CameraConfiguration::validate()``
function. If the new values are not supported by the ``Camera`` device, the
validation process adjusts the parameters to what it considers supported values.

The ``validate`` method returns a `Status
<http://libcamera.org/api-html/classlibcamera_1_1CameraConfiguration.html#a64163f21db2fe1ce0a6af5a6f6847744>`_
which applications shall check to see if the Pipeline Handler adjusted the
configuration.

For example, the code above set the width and height to 640x480, but if
the camera cannot produce an image that large, it might adjust the
configuration to the supported size of 320x240 and return ``Adjusted`` as
validation status result.

If the configuration to validate cannot be adjusted to a set of supported
values, the validation procedure fails and returns the ``Invalid`` status.

For this example application, the code below prints the adjusted values
to standard out.

.. code:: cpp

   config->validate();
   std::cout << "Validated viewfinder configuration is: " << streamConfig.toString() << std::endl;

For example, the output might be something like
``Validated viewfinder configuration is: 320x240-MJPEG``

A validated ``CameraConfiguration`` can bet given to the ``Camera`` device to
be applied to the system.

.. code:: cpp

   camera->configure(config.get());

If an application doesn’t first validate the configuration before
calling ``Camera::configure()``, there’s a chance that calling the function
fails.

Allocate FrameBuffers
---------------------

An application needs to reserve the memory that libcamera can write incoming
frames and data to, and that the application can then read. The libcamera
library uses ``FrameBuffer`` instances to represent memory buffer allocated in
memory. An application should reserve enough memory for the frame size the
streams need based on the configured image sizes and formats.

The libcamera library consumes buffers provided by applications as
``FrameBuffer`` instances, which makes libcamera a consumer of buffers
exported by other devices (such as displays or video encoders), or
allocated from an external allocator (such as ION on Android).

In some situations, applications do not have any means to allocate or
get hold of suitable buffers, for instance, when no other device is
involved, or on Linux platforms that lack a centralized allocator. The
``FrameBufferAllocator`` class provides a buffer allocator an
application can use in these situations.

An application doesn’t have to use the default ``FrameBufferAllocator``
that libcamera provides. It can instead allocate memory manually and
pass the buffers in ``Request``\s (read more about ``Request`` in
`the frame capture section <#frame-capture>`_ of this guide). The
example in this guide covers using the ``FrameBufferAllocator`` that
libcamera provides.

Using the libcamera ``FrameBufferAllocator``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Applications create a ``FrameBufferAllocator`` for a Camera and use it
to allocate buffers for streams of a ``CameraConfiguration`` with the
``allocate()`` function.

The list of allocated buffers can be retrieved using the ``Stream`` instance
memory has been reserved for as the parameter of the
``FrameBufferAllocator::buffers()`` function.

.. code:: cpp

   FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);

   for (StreamConfiguration &cfg : *config) {
       int ret = allocator->allocate(cfg.stream());
       if (ret < 0) {
           std::cerr << "Can't allocate buffers" << std::endl;
           return -ENOMEM;
       }

       unsigned int allocated = allocator->buffers(cfg.stream()).size();
       std::cout << "Allocated " << allocated << " buffers for stream" << std::endl;
   }

Frame Capture
~~~~~~~~~~~~~

The libcamera library implements a streaming model based on per-frame requests.
For each frame an application wants to capture it must queue a request for it to
the camera. With libcamera, a ``Request`` is at least one ``Stream`` associated
with a ``FrameBuffer`` representing the memory location where frames have to be
stored.

First, by using the ``Stream`` instance associated to each
``StreamConfiguration``, retrieve the list of ``FrameBuffer``\s created by for
it by the above presented frame allocator. Then create a vector of requests to
be submitted to the camera.

.. code:: cpp

   Stream *stream = streamConfig.stream();
   const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
   std::vector<Request *> requests;

Proceed to fill the request vector by creating ``Request`` instances from the
camera device, and associate in each of them the ``Stream`` with one of the
buffers allocated for it by the ``FrameBufferAllocator``.

.. code:: cpp

       for (unsigned int i = 0; i < buffers.size(); ++i) {
           Request *request = camera->createRequest();
           if (!request)
           {
               std::cerr << "Can't create request" << std::endl;
               return -ENOMEM;
           }

           const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
           int ret = request->addBuffer(stream, buffer.get());
           if (ret < 0)
           {
               std::cerr << "Can't set buffer for request"
                     << std::endl;
               return ret;
           }

           requests.push_back(request);
       }

.. TODO: Controls

.. TODO: A request can also have controls or parameters that you can apply to the image.

Event handling and callbacks
----------------------------

The libcamera library uses the concept of signals and slots (`similar to Qt
<https://doc.qt.io/qt-5/signalsandslots.html>`_) to connect events with
callbacks to handle them.

The ``Camera`` device emits two signals that applications can connect to in
order to execute callbacks on frame completion events.

The ``Camera::bufferCompleted`` signal notifies to applications when a buffer
with image data is available. Receiving notifications about the single buffer
completion event allows applications to implement partial request completion
support, and to inspect the buffer content before the request it is part of has
completed.

The ``Camera::requestCompleted`` signal notifies to applications when a request
has completed, which means all the buffers the request contains have now
completed. Request completion notifications are always emitted in the same order
as the request have been queued to the camera.

To receive the signals emission notifications, connect a slot function to the
signal to handle it in the application code.

.. code:: cpp

   camera->requestCompleted.connect(requestComplete);

For this example application, only the ``Camera::requestCompleted`` signal gets
handled and matching ``requestComplete`` slot method outputs information about
the FrameBuffer to standard outpuit. This callback is typically where an
application accesses the image data from the camera and does something with it.

Signals operate in the libcamera ``CameraManager`` thread context, so it
is important not to block the thread for a long time, as this blocks
internal processing of the camera pipelines, and can affect realtime
performance.

Handle request completion events
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create the ``requestComplete`` function by matching the slot signature:

.. code:: cpp

   static void requestComplete(Request *request)
   {
       // Code to follow
   }

Request completion events might be emitted for actually canceled requests
caused, for example, by unexpected application shutdown. To avoid an application
processing invalid image data, it’s worth checking that the
request has completed successfully. The list of request completion statuses is
available in the `Request class documentation
<https://www.libcamera.org/api-html/classlibcamera_1_1Request.html#a2209ba8d51af8167b25f6e3e94d5c45b>`_.

.. code:: cpp

   if (request->status() == Request::RequestCancelled) return;

If the ``Request`` has completed successfully, applications can access the
completed buffers using the ``Request::buffers()`` function, which returns a map
of ``FrameBuffer`` instances associated with the ``Stream`` that produced the
images.

.. code:: cpp

   const std::map<Stream *, FrameBuffer *> &buffers = request->buffers();

Iterating through the map allows applications to inspect each completed buffer
in this request, and access the metadata associated to each frame.

The metadata buffer contains information such the capture status, a timestamp,
and the bytes used, as described in the `FrameMetadata documentation
<http://libcamera.org/api-html/structlibcamera_1_1FrameMetadata.html>`_.

.. code:: cpp

   for (auto bufferPair : buffers) {
       FrameBuffer *buffer = bufferPair.second;
       const FrameMetadata &metadata = buffer->metadata();
   }

For this example application, inside the ``for`` loop from above, print
the Frame sequence number and details of the planes.

.. code:: cpp

   std::cout << " seq: " << std::setw(6) << std::setfill('0') << metadata.sequence << " bytesused: ";

   unsigned int nplane = 0;
   for (const FrameMetadata::Plane &plane : metadata.planes)
   {
       std::cout << plane.bytesused;
       if (++nplane < metadata.planes.size()) std::cout << "/";
   }

   std::cout << std::endl;

The expected output shows each monotonically increasing frame sequence
number and the bytes used by planes.

.. code:: text

   seq: 000000 bytesused: 1843200
   seq: 000002 bytesused: 1843200
   seq: 000004 bytesused: 1843200
   seq: 000006 bytesused: 1843200
   seq: 000008 bytesused: 1843200
   seq: 000010 bytesused: 1843200
   seq: 000012 bytesused: 1843200
   seq: 000014 bytesused: 1843200
   seq: 000016 bytesused: 1843200
   seq: 000018 bytesused: 1843200
   seq: 000020 bytesused: 1843200
   seq: 000022 bytesused: 1843200
   seq: 000024 bytesused: 1843200
   seq: 000026 bytesused: 1843200
   seq: 000028 bytesused: 1843200
   seq: 000030 bytesused: 1843200
   seq: 000032 bytesused: 1843200
   seq: 000034 bytesused: 1843200
   seq: 000036 bytesused: 1843200
   seq: 000038 bytesused: 1843200
   seq: 000040 bytesused: 1843200
   seq: 000042 bytesused: 1843200

A completed buffer contains of course image data which can be accessed through
the per-plane dma-buf file descriptor transported by the ``FrameBuffer``
instance. An example of how to write image data to disk is available in the
`BufferWriter class
<https://git.linuxtv.org/libcamera.git/tree/src/cam/buffer_writer.cpp>`_ part of
the utility ``cam`` application in the libcamera repository.

With the handling of this request completed, it is possible to re-use the
buffers by adding them to a new ``Request`` instance with their matching
streams, and finally, queue the new capture request to the camera device to

.. code:: cpp

   request = camera->createRequest();
   if (!request)
   {
       std::cerr << "Can't create request" << std::endl;
       return;
   }

   for (auto it = buffers.begin(); it != buffers.end(); ++it)
   {
       Stream *stream = it->first;
       FrameBuffer *buffer = it->second;

       request->addBuffer(stream, buffer);
   }

   camera->queueRequest(request);

Request queueing
----------------

The ``Camera`` device is now ready to receive frame capture requests and
actually start delivering frames.

In order to prepare for that an application needs to first start the camera,
and then queue requests to it for them to being processed.

In the main() function, just after having connected the
``Camera::requestCompleted`` signal to the callback handler, start the camera
and queue all the previously created requests.

.. code:: cpp

   camera->start();
   for (Request *request : requests)
       camera->queueRequest(request);

Start an event loop
~~~~~~~~~~~~~~~~~~~

The libcamera library needs an event loop to monitor and dispatch events
generated by the video devices part of the capture pipeline. Libcamera provides
its own ``EventDispatcher`` class (inspired by `the Qt event system
<https://doc.qt.io/qt-5/eventsandfilters.html>`_) to process and deliver events
generated by ``EventNotifiers``.

The libcamera library implements this by creating instances of the
``EventNotifier`` class, which models a file descriptor event source
registered to an ``EventDispatcher``. Whenever the ``EventDispatcher`` detects
an event on a notifier it is monitoring it emits the notifier's
``EventNotifier::activated`` signal. The libcamera components connect to the
notifiers' signals and emit application visible events, such as the
``Camera::bufferReady`` and ``Camera::requestCompleted`` ones.

The code below retrieve a reference to the system-wide event dispatcher and for
the duration of a fixed 3 seconds timeout processes all the events detected in
the system.

.. code:: cpp

   EventDispatcher *dispatcher = cm->eventDispatcher();
   Timer timer;
   timer.start(3000);
   while (timer.isRunning())
       dispatcher->processEvents();

Clean up and stop the application
---------------------------------

The application is now finished with the camera and the resources the
camera uses, so needs to do the following:

-  stop the camera
-  free the buffers in the FrameBufferAllocator and delete it
-  release the lock on the camera and reset the pointer to it
-  stop the camera manager

.. code:: cpp

   camera->stop();
   allocator->free(stream);
   delete allocator;
   camera->release();
   camera.reset();
   cm->stop();

   return 0;

Build and run instructions
--------------------------

To build the application use the `Meson build system <https://mesonbuild.com/>`_
which is also the official build system of the libcamera library.

Make sure both ``meson`` and ``libcamera`` are installed in your system. Please
refer to your distribution documentation to install meson and install the most
recent version of libcamera from the git repository at `Linux TV
<https://git.linuxtv.org/libcamera.git/>_`. You would also need to install the
``pkgconfig`` tool to correctly identify the libcamera.so object install
location in the system, as here explained.

Dependencies
~~~~~~~~~~~~

The test application here presented depends on the library to be available in a
path that meson can identify. The libcamera install procedure performed using
the ``ninja install`` command might end up deploying the libcamera components in
the ``/usr/local/`` path, depending on your distribution. In order to instruct
meson to look into that path when searching for ``libcamera.so`` it is required
to point the ``PKG_CONFIG_PATH`` environment variable to the right location.

Adjust the following command to use the ``pkgconfig`` directory where
libcamera has been installed in your system.

.. code:: shell

   export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/

Verify that ``pkgconfig`` can identify the ``camera`` library with

.. code:: shell

   $ pkg-config --libs camera --cflags
     -I/usr/local/include/libcamera -L/usr/local/lib -lcamera

``meson`` can alternatively use ``cmake`` to locate packages, please refer to
the ``meson`` documentation if you prefer to use it in place of ``pkgconfig``

Build file
~~~~~~~~~~

With the dependencies correctly identified, prepare a ``meson.build`` build file
to be placed in the same directory where the application lives. You can
name your application as you like, but be sure to update the following snippet
accordingly. In this example, the application file has been named
``simpler-cam.cpp``.

.. code::

   project('simpler-cam', 'cpp')

   simpler_cam = executable('simpler-cam',
       'simpler-cam.cpp',
       dependencies: dependency('camera', required : true))

The ``dependencies`` line instructs meson to ask ``pkgconfig`` (or ``cmake``) to
locate the ``camera`` library, which the test application dynamically links
against.

With the build file in place, compile and run the application with:

.. code:: shell

   $ meson build
   $ cd build
   $ ninja
   $ ./simpler-cam

It is possible to increase the library debug output by using environment
variables which control the library log filtering system:

.. code:: shell

   $ LIBCAMERA_LOG_LEVELS=0 ./simpler-cam
