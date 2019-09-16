#
# libproxy - Helper library for IPA proxy workers
#
lib_proxy_sources = files([
    'event_loop.cpp',
])

libproxy = static_library('proxy', lib_proxy_sources,
                          include_directories : libcamera_internal_includes,
                          dependencies : libcamera_dep)

#
# Proxy workers
#
ipa_proxy_workers = [
    [
        'ipa_proxy_linux',
        files([
            'ipa_proxy_linux_worker.cpp',
        ]),
    ],
]

proxy_install_dir = join_paths(get_option('libexecdir'), 'libcamera')

foreach worker : ipa_proxy_workers
    executable(worker[0], worker[1],
               include_directories : libcamera_internal_includes,
               link_with : libproxy,
               install : true,
               install_dir : proxy_install_dir,
               dependencies : libcamera_dep)
endforeach

config_h.set('IPA_PROXY_DIR',
             '"' + join_paths(get_option('prefix'), proxy_install_dir) + '"')
