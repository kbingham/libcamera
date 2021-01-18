# SPDX-License-Identifier: CC0-1.0

project('ia_imaging', 'c')

ia_imaging_source = meson.current_source_dir() / 'ia_imaging'

ia_library_path = ia_imaging_source / 'usr/lib64'

ia_library_includes = include_directories([
  'ia_imaging/usr/include',
  'ia_imaging/usr/include/ia_imaging',
])

cc = meson.get_compiler('c')

ia_deps = [
  cc.find_library('libia_aiq', dirs : ia_library_path),
  cc.find_library('libia_exc', dirs : ia_library_path),
  cc.find_library('libia_coordinate', dirs : ia_library_path),
  cc.find_library('libia_nvm', dirs : ia_library_path),
  cc.find_library('libia_cmc_parser', dirs : ia_library_path),
  cc.find_library('libia_mkn', dirs : ia_library_path),
  cc.find_library('libia_log', dirs : ia_library_path),
  cc.find_library('libSkyCamAICKBL', dirs : ia_library_path),
]

ia_so_files = files([
  ia_library_path / 'libia_aiq.so',
  ia_library_path / 'libia_exc.so',
  ia_library_path / 'libia_coordinate.so',
  ia_library_path / 'libia_nvm.so',
  ia_library_path / 'libia_cmc_parser.so',
  ia_library_path / 'libia_mkn.so',
  ia_library_path / 'libia_log.so',
])

install_data(ia_so_files,
             install_dir : get_option('libdir'))

ia_imaging_dep = declare_dependency(
    dependencies : ia_deps,
    include_directories : ia_library_includes,
)

# find ia_imaging/usr/include/ | sort | while read line; do echo "    '$line',"; done;
