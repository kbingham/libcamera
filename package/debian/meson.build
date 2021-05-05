# SPDX-License-Identifier: CC0-1.0

# Install debian package information to the root directory as /DEBIAN.

# This should only be used by installing to a custom DESTDIR and then calling
# dpkg -b to produce the debian package binary.
# For example:
#
#   ninja -C install DESTDIR=build/package/libcamera
#   dpkg -b build/package/libcamera

debian_pkg_sources = files([
  'control',
  ])

install_data(debian_pkg_sources,
             install_dir : '/DEBIAN/')
