# meson.build for ecmd-plugin
# Collect all source files
ecmd_plugin_src = files(
  'ecmd_dll_cfam.cpp',
  'ecmd_dll_init.cpp',
  'ecmd_dll_istep.cpp',
  'ecmd_dll_scom.cpp',
  'ecmd_util.cpp',
)

# Include directories
ecmd_plugin_inc = include_directories('.')

# Find dependent libraries
ipl_lib       = cxx.find_library('ipl', required: true)
ecmd_lib      = cxx.find_library('ecmd', required: true)

dep_targeting = dependency('libtargeting', required: true)
dep_hwaccess = dependency('libhwaccess', required: true)
dep_targetsvc = dependency('libtargetsvc', required: true)
dep_transport = dependency('libtransport', required: true)


# Combine them in a list for linking
ecmd_plugin_deps = [
  ipl_lib,
  ecmd_lib,
  dep_targeting,
  dep_hwaccess,
  dep_targetsvc,
  dep_transport
]

# ---------------------------------------------------------------------
# Build the shared library
# ---------------------------------------------------------------------
ecmd_plugin_lib = shared_library(
  'ecmd-plugin',                    # will produce libecmd-plugin.so
  ecmd_plugin_src,
  include_directories: ecmd_plugin_inc,
  dependencies: ecmd_plugin_deps,
  install: true,
  install_dir: get_option('libdir'),
)

ecmd_plugin_dep = declare_dependency(
    link_with: ecmd_plugin_lib,
    include_directories: ecmd_plugin_inc,
    dependencies: ecmd_plugin_deps
)

# ---------------------------------------------------------------------
# pkg-config file generation
# ---------------------------------------------------------------------
pkgconfig = import('pkgconfig')

pkgconfig.generate(
  libraries: ecmd_plugin_lib,        # <— use the variable returned above
  version: meson.project_version(),
  filebase: 'ecmd-plugin',
  name: 'ecmd-plugin',
  description: 'eCMD platform plugin shared library',
  subdirs: 'ecmd',
  requires: [
    'ipl',
    'targeting',
    'targetsvc',
    'ecmd',
  ],
)

