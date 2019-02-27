vsp1_sources = files([
    'vsp1.cpp',
    'main.cpp',
    'options.cpp',
])

vsp1_includes = [
    libcamera_includes,
    libcamera_internal_includes,
]

vsp1  = executable('vsp1', vsp1_sources,
                  link_with : libcamera,
                  include_directories : vsp1_includes,
                  install : true)
