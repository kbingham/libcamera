# SPDX-License-Identifier: CC0-1.0

# Handle system paths
datadir = get_option('datadir')
libdir = get_option('libdir')
prefix = get_option('prefix')
sysconfdir = get_option('sysconfdir')

libcamera_datadir = datadir / 'libcamera'
libcamera_libdir = libdir / 'libcamera'
libcamera_sysconfdir = sysconfdir / 'libcamera'

config_h.set('LIBCAMERA_SYSCONF_DIR', '"' + prefix / libcamera_sysconfdir + '"')
config_h.set('LIBCAMERA_DATA_DIR', '"' + prefix / libcamera_datadir + '"')

# Module Signing
openssl = find_program('openssl', required : true)
if openssl.found()
    ipa_priv_key = custom_target('ipa-priv-key',
                                 output : [ 'ipa-priv-key.pem' ],
                                 command : [ gen_ipa_priv_key, '@OUTPUT@' ])
    config_h.set('HAVE_IPA_PUBKEY', 1)
    ipa_sign_module = true
else
    ipa_sign_module = false
endif

# The Android HAL must be built before, and is included in libcamera.
subdir('android')

subdir('libcamera')
subdir('ipa')

subdir('cam')
subdir('qcam')

subdir('v4l2')
subdir('gstreamer')
