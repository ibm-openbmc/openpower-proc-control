# SPDX-License-Identifier: Apache-2.0

installdir = join_paths(get_option('libexecdir'), 'openpower-proc-control')

cfam_deps = [
      sdbusplus_dep,
      pdi_dep,
      phosphor_logging_dep,
]

sources = [
    'application.cpp',
    'cfam_access.cpp',
    'cfam_fields.cpp',
    'cfam_main.cpp',
    'local_bmc.cpp',
    'local_cfam.cpp',
    'sibling_cfam.cpp',
    'sibling_bmc.cpp',
    'services.cpp',
    'sysfs.cpp',
]

executable('rbmc-cfamd',
    sources,
    dependencies: [
      cfam_deps
    ],
    install: true,
    install_dir: installdir,
)

executable('cfamstool',
    'cfam_access.cpp',
    'cfam_fields.cpp',
    'cfams_tool.cpp',
    'local_cfam.cpp',
    'sysfs.cpp',
    dependencies: [cfam_deps, cli11_dep],
    install: true
)
