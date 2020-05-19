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

cam  = executable('cam', cam_sources,
                  dependencies : [ libatomic, libcamera_dep ],
                  install : true)
