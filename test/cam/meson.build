# SPDX-License-Identifier: CC0-1.0

if not cam_enabled
  subdir_done()
endif

cam_tests = files('capture-test.sh')
cam_tests = [
    'capture-test.sh',
    'valgrind-cam.sh',
]

foreach script : cam_tests
    test(script, files(script),
        args : cam,
        suite : 'integration',
        is_parallel : false,
        timeout : 60)
endforeach

