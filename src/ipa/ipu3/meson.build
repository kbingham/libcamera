# SPDX-License-Identifier: CC0-1.0

subdir('data')

ipa_name = 'ipa_ipu3'

ipu3_ipa_deps = [
    libatomic,
    libcamera_dep,
    ia_imaging_dep,
]

ipu3_ipa_includes = [
    ipa_includes,
    libipa_includes,
]

ipu3_ipa_files = files([
    'ipu3.cpp',
])

subdir('aic')
subdir('aiq')
subdir('stats')

mod = shared_module(ipa_name,
                    [ipu3_ipa_files, libcamera_generated_ipa_headers],
                    name_prefix : '',
                    include_directories : ipu3_ipa_includes,
                    dependencies : ipu3_ipa_deps,
                    link_with : libipa,
                    install : true,
                    install_dir : ipa_install_dir)

if ipa_sign_module
    custom_target(ipa_name + '.so.sign',
                  input : mod,
                  output : ipa_name + '.so.sign',
                  command : [ipa_sign, ipa_priv_key, '@INPUT@', '@OUTPUT@'],
                  install : false,
                  build_by_default : true)
endif

config_h.set('MACRO_KBL_AIC', 1)
