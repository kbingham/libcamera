-   What do I as a reader need to do and know to write an IPA for my new platform
     (which I have conveniently already written the pipeline handler for)

-   How do we make a plan for IPA.

-   Aim for an outline for IPA.

-   Creating the skeleton IPA
    -   building, and compiling

-   Process isolations, and the difference between open source and closed source IPA.

-   Communicating between pipeline handler and IPA
    -   Serialisation between processes. (talking about, not implementing, the libcamera core implement it.)

-   Existing current templates to start from (VIMC, RKISP1, RPi IPA)
    -   So we might want to generate a combination of VIMC/RKISP into the 'Vivid-IPA'.

* * *

# Building an IPA module

## What is an IPA module

Generally written as part of the pipeline handler, the IPA module manager looks for a module matching the handler. You add matching modules by creating an appropriate folder in _src/ipa_, adding the source to the build system, and searching for a match in the corresponding pipeline handler.

## Module isolation

The IPA module manager discovers IPA modules from disk, queries and loads them, and creates IPA contexts. It supports isolation of the modules in a separate process with IPC communication and offers a unified IPAInterface view of the IPA contexts to pipeline handlers regardless of whether the modules are isolated or loaded in the same process.

Module isolation is based on the module licence. Open-source modules are loaded without isolation, while closed-source module are forcefully isolated. The isolation mechanism ensures that no code from a closed-source module is ever run in the libcamera process.

All interactions with the IPA context go the same interface regardless of process isolation. In all cases the data passed to the IPAInterface methods is serialized to Plain Old Data, either for the purpose of passing it to the IPA context plain C API, or to transmit the data to the isolated process through IPC.

## Creating a skeleton IPA module

-   Create an instance of the `IPAInterface` base class
-   create class member methods: `init`, `start`, `stop` etc
    -   `init` - set values with config, can be in method or file (if so, add file to build)
    -   `start`/`stop` - prepare/release resources, which are what?
    -   `configure` - Called by Pipeline handler to configure the IPA stream and sensor settings.
    -   `map`/`unmap` buffers - inform ipa module of the buffers shared from the pipeline handler
    -   `process` - process by which pipeline handler informs a module of events in an on-going capture operation. The events you define in the pipeline handler
    -   `queueframeaction` - Queue an action associated with a frame to the pipeline handler.
-   any private methods you need

## Create an IPA context

-   Create in camera data during pipeline handler match process
-   Start and stop IPA module generally when starting and stopping streams

<!-- 
What is a module?
    - Inside _src/ipa_, manager looks for matching module and configuration.
    - Add (code and conf) to build system (folder for IPA, and meson file one level above)
    - Can also include headers, for what exactly? -->

The IPA module manager discovers IPA modules from disk, queries and loads them, and creates IPA contexts. It supports isolation of the modules in a separate process with IPC communication and offers a unified IPAInterface view of the IPA contexts to pipeline handlers regardless of whether the modules are isolated or loaded in the same process.

Module isolation is based on the module licence. Open-source modules are loaded without isolation, while closed-source module are forcefully isolated. The isolation mechanism ensures that no code from a closed-source module is ever run in the libcamera process.

To create an IPA context, pipeline handlers call the IPAManager::ipaCreate() method. For a directly loaded module, the manager calls the module's ipaCreate() function directly and wraps the returned context in an IPAContextWrapper that exposes an IPAInterface.

\+---------------+
|   Pipeline    |
|    Handler    |
\+---------------+
        \|
        v
\+---------------+                   +---------------+
|      IPA      |                   |  Open Source  |
|   Interface   |                   |  IPA Module   |
\| - - - - - - - \|                   \| - - - - - - - \|
|  IPA Context  |  ipa_context_ops  |  ipa_context  |
|    Wrapper    | ----------------> |               |
\+---------------+                   +---------------+

For an isolated module, the manager instantiates an IPAProxy which spawns a new process for an IPA proxy worker. The worker loads the IPA module and creates the IPA context. The IPAProxy alse exposes an IPAInterface.

\+---------------+                   +---------------+
|   Pipeline    |                   | Closed Source |
|    Handler    |                   |  IPA Module   |
\+---------------+                   | - - - - - - - |
        |                           |  ipa_context  |
        v                           |               |
\+---------------+                   +---------------+
|      IPA      |           ipa_context_ops ^
|   Interface   |                           |
| - - - - - - - |                   +---------------+
|   IPA Proxy   |     operations    |   IPA Proxy   |
|               | ----------------> |    Worker     |
\+---------------+      over IPC     +---------------+

The IPAInterface implemented by the IPAContextWrapper or IPAProxy is returned to the pipeline handler, and all interactions with the IPA context go the same interface regardless of process isolation.

In all cases the data passed to the IPAInterface methods is serialized to Plain Old Data, either for the purpose of passing it to the IPA context plain C API, or to transmit the data to the isolated process through IPC.

---

# IPA module writers guide

Image processing algorithm (IPA) modules form part of a device-specific pipeline handler to provide support for image enhancement. At a minimum, these should include auto exposure, gain, and white balance, plus auto focus if a camera device has a focus lens, and other optional image enhancement algorithms.

<!-- TODO: More here -->

## Prerequisite knowledge

<!-- TODO: Add here -->

## Creating a PipelineHandler

This guide walks through the steps to create a simple IPA module that works alongside the Vivid pipeline handler that supports the [V4L2 Virtual Video Test Driver (vivid)](https://www.kernel.org/doc/html/latest/admin-guide/media/vivid.html).

To use the vivid test driver, you first need to check that the vivid kernel module is loaded
with the ``modprobe vivid`` command, and have followed and implemented the pipeline handler writers guide.

## Create the skeleton file structure

To add a new IPA module, create a directory to hold the module code
in the *src/ipa/* directory that matches the name of the
pipeline (in this case *vivid*). Inside the new directory add a *meson.build* file that integrates 
with the libcamera build system, and a *vivid.cpp* file that matches the name of the module.

In the *meson.build* file, add the *vivid.cpp* file as a shared module build source for libcamera. To allow for optional module signing (defined at build time), add a custom target for signed shared object file.

<!-- TODO: The above need more, and does this build the module even if there is no pipeline? Unlikely I know -->

```meson
# SPDX-License-Identifier: CC0-1.0

ipa_name = 'ipa_vivid'

mod = shared_module(ipa_name,
                    'vivid.cpp',
                    name_prefix : '',
                    include_directories : [ipa_includes, libipa_includes],
                    dependencies : libcamera_dep,
                    link_with : libipa,
                    install : true,
                    install_dir : ipa_install_dir)

if ipa_sign_module
    custom_target(ipa_name + '.so.sign',
                  input : mod,
                  output : ipa_name + '.so.sign',
                  command : [ ipa_sign, ipa_priv_key, '@INPUT@', '@OUTPUT@' ],
                  install : false,
                  build_by_default : true)
endif
```

Users of libcamera can selectively enable pipelines while building libcamera using the 
``pipelines`` option. 

For example, to enable only the IPU3, UVC, and VIVID pipelines, specify them as a comma separated
list with ``-Dpipelines`` when generating a build directory:

.. code-block:: shell

    meson build -Dpipelines=ipu3,uvcvideo,vivid'

`Read the Meson build configuration documentation <https://mesonbuild.com/Configuring-a-build-directory.html>`_ for more information.

To ensure the build system builds an IPA module matching a pipeline handler you also need to add it as a potential IPA module to the _ipa/meson.build_ file.

Add the name of your module to the `ipas` array of options.

```meson
ipas = ['raspberrypi', 'rkisp1', 'vimc', 'vivid']
```

In *vivid.cpp* add the module to the `libcamera` namespace, create an instance of the [IPAInterface](http://libcamera.org/api-html/classlibcamera_1_1IPAInterface.html)
base class, and add stub methods for the overridden class members.

```cpp
namespace libcamera {

class IPAVivid : public IPAInterface
{
public:
	IPAVivid();
	~IPAVivid();

	int init(const IPASettings &settings) override;

	int start() override;
	void stop() override;

	void configure(const CameraSensorInfo &sensorInfo,
		       const std::map<unsigned int, IPAStream> &streamConfig,
		       const std::map<unsigned int, const ControlInfoMap &> &entityControls,
		       const IPAOperationData &ipaConfig,
		       IPAOperationData *result) override {}
	void mapBuffers(const std::vector<IPABuffer> &buffers) override {}
	void unmapBuffers(const std::vector<unsigned int> &ids) override {}
	void processEvent(const IPAOperationData &event) override {}

private:

};
} /* namespace libcamera */
```

For debugging and testing a module during development, you can
define a log message category. The
`LOG_DEFINE_CATEGORY` and `LIBCAMERA_LOG_LEVELS` macros are part of
[the inbuilt libcamera logging
infrastructure](http://libcamera.org/api-html/log_8h.html) that allow
you to inspect internal operations in a user-configurable way.

Add the following before the `IPAVivid` class declaration.

```cpp
LOG_DEFINE_CATEGORY(IPAVivid)
```

At this point you need the following includes to the top of the file.

```cpp
#include "libcamera/internal/log.h"
#include <libcamera/ipa/ipa_interface.h>
```

## Add to Pipeline handler

To integrate a…


In the pipeline handler `start` method, above the `	ret = data->video_->streamOn();` line…

```cpp
ret = data->ipa_->start();
if (ret) {
    data->video_->releaseBuffers();
    return ret;
}
```

Above `data->video_->releaseBuffers();`, add `data->ipa_->stop();`.

Add explanation…

In the `stop` method, above `data->video_->releaseBuffers();`, add.

```cpp
data->ipa_->stop();
```

After a sucessful device match in the `match` method, before the `if (data->init())` line create…

```cpp
data->ipa_ = IPAManager::createIPA(this, 0, 0);
if (data->ipa_ != nullptr) {
std::string conf = data->ipa_->configurationFile("vimc.conf");
data->ipa_->init(IPASettings{ conf });
} else {
LOG(VIMC, Warning) << "no matching IPA found";
}
```