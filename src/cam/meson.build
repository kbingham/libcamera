# SPDX-License-Identifier: CC0-1.0

cam_sources = files([
    'capture.cpp',
    'event_loop.cpp',
    'file_sink.cpp',
    'frame_sink.cpp',
    'main.cpp',
    'options.cpp',
    'stream_options.cpp',
])

cam_cpp_args = []

libdrm = dependency('libdrm', required : false)

if libdrm.found()
cam_cpp_args += [ '-DHAVE_KMS' ]
cam_sources += files([
    'drm.cpp',
])
endif

cam  = executable('cam', cam_sources,
                  dependencies : [ libatomic, libcamera_dep, libdrm ],
                  cpp_args : cam_cpp_args,
                  install : true)
