# SPDX-License-Identifier: Apache-2.0
test(
    'cfam-tests',
    executable(
        'cfam-tests',
        'cfam_access_test.cpp',
        'cfam_fields_test.cpp',
        'local_cfam_test.cpp',
        'sysfs_test.cpp',
        '../src/cfam_access.cpp',
        '../src/cfam_fields.cpp',
        '../src/local_cfam.cpp',
        '../src/sysfs.cpp',
        dependencies: [gmock_dep, gtest_dep, phosphor_logging_dep],
        include_directories: '../src',
    ),
)
