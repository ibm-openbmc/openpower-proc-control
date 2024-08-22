# SPDX-License-Identifier: Apache-2.0

installdir = join_paths(get_option('libexecdir'), 'openpower-proc-control')

cfam_deps = [sdbusplus_dep, pdi_dep, phosphor_logging_dep]

sources = [
    'application.cpp',
    'cfam_access.cpp',
    'cfam_fields.cpp',
    'cfam_main.cpp',
    'sysfs.cpp',
]

executable(
    'rbmc-cfamd',
    sources,
    dependencies: [cfam_deps],
    install: true,
    install_dir: installdir,
)
