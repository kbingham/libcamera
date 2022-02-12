# SPDX-License-Identifier: CC0-1.0

gtk4_dep = dependency('gtk4',
                      method : 'pkg-config',
                      modules : ['Core', 'Gui', 'Widgets'],
                      required : get_option('pinhole'),
                      version : '>=4.0')

if not gtk4_dep.found()
    pinhole_enabled = false
    subdir_done()
endif

pinhole_enabled = true

pinhole_cpp_args = []

pinhole_sources = files([
    'main.cpp',
    'main_window.cpp',
])

pinhole_deps = [
    libcamera_public,
    gtk4_dep,
]

pinhole  = executable('pinhole', pinhole_sources,
                      install : true,
                      dependencies : pinhole_deps,
                      cpp_args : pinhole_cpp_args)
