project(
    'openpower-proc-control',
    'cpp',
    default_options: [
        'warning_level=3',
        'werror=true',
        'cpp_std=c++23',
        'buildtype=debugoptimized',
        'b_ndebug=if-release',
    ],
    license: 'Apache-2.0',
    version: '1.0',
    meson_version: '>=1.1.1',
)
add_project_arguments('-Wno-psabi', language: 'cpp')

if get_option('cpp_std') != 'c++23'
    error('This project requires c++23')
endif

if (get_option('buildtype') == 'minsize')
    add_project_arguments('-DNDEBUG', language: 'cpp')
endif

cxx = meson.get_compiler('cpp')

extra_sources = []
extra_dependencies = []
extra_unit_files = []

# Configuration header file(config.h) generation
conf_data = configuration_data()

conf_data.set_quoted(
    'DEVTREE_EXPORT_FILTER_FILE',
    get_option('DEVTREE_EXPORT_FILTER_FILE'),
    description: 'Path to the phal devtree export filter file',
)

conf_data.set_quoted(
    'DEVTREE_EXP_FILE',
    get_option('DEVTREE_EXP_FILE'),
    description: 'Path to the devtree export copy file',
)

conf_data.set_quoted(
    'CEC_DEVTREE_RW_PATH',
    get_option('CEC_DEVTREE_RW_PATH'),
    description: 'Path to the devtree file r/w version',
)

conf_data.set_quoted(
    'CEC_DEVTREE_RO_BASE_PATH',
    get_option('CEC_DEVTREE_RO_BASE_PATH'),
    description: 'Base path to the devtree file read only version',
)
conf_data.set_quoted(
    'CEC_INFODB_PATH',
    get_option('CEC_INFODB_PATH'),
    description: 'Path to the devtree attributes based database path',
)

conf_data.set_quoted(
    'DEVTREE_REINIT_ATTRS_LIST',
    get_option('DEVTREE_REINIT_ATTRS_LIST'),
    description: 'Path to the phal devtree reinit attribute list file',
)

conf_data.set_quoted(
    'OP_DUMP_OBJ_PATH',
    get_option('op_dump_obj_path'),
    description: 'Object path requesting OpenPOWER dumps',
)

configure_file(configuration: conf_data, output: 'config.h')

unit_subs = configuration_data()
unit_subs.set('bindir', join_paths(get_option('prefix'), get_option('bindir')))
unit_subs.set('ENABLE_PHAL_TRUE', '#')
unit_subs.set(
    'CEC_DEVTREE_RW_PATH',
    get_option('CEC_DEVTREE_RW_PATH'),
    description: 'Path to the devtree file r/w version',
)

if get_option('phal').enabled() and get_option('p9').enabled()
    error('phal and p9 cannot be selected at the same time')
endif

build_p9 = not get_option('p9').disabled()
build_openfsi = not get_option('openfsi').disabled()
build_phal = get_option('phal').enabled()

if get_option('phal').enabled() and not get_option('p9').disabled()
    build_p9 = false
endif

summary('building p9', build_p9)
summary('building openfsi', build_openfsi)
summary('building phal', build_phal)

if build_p9
    extra_sources += [
        'procedures/p9/cleanup_pcie.cpp',
        'procedures/p9/set_sync_fsi_clock_mode.cpp',
        'procedures/p9/start_host.cpp',
        'procedures/p9/start_host_mpreboot.cpp',
        'procedures/p9/enter_mpreboot.cpp',
        'procedures/p9/thread_stopall.cpp',
    ]
endif
if build_openfsi
    extra_sources += ['procedures/openfsi/scan.cpp']
endif
if build_phal
    extra_sources += [
        'procedures/phal/start_host.cpp',
        'procedures/phal/set_SPI_mux.cpp',
        'procedures/phal/proc_pre_poweroff.cpp',
        'procedures/phal/check_host_running.cpp',
        'procedures/phal/import_devtree.cpp',
        'procedures/phal/enter_mpreboot.cpp',
        'procedures/phal/reinit_devtree.cpp',
        'procedures/phal/thread_stopall.cpp',
        'extensions/phal/common_utils.cpp',
        'extensions/phal/pdbg_utils.cpp',
        'extensions/phal/create_pel.cpp',
        'extensions/phal/phal_error.cpp',
        'extensions/phal/dump_utils.cpp',
        'temporary_file.cpp',
        'util.cpp',
    ]
    extra_dependencies += [
        dependency('libdt-api'),
        cxx.find_library('ekb'),
        cxx.find_library('ipl'),
        cxx.find_library('phal'),
        cxx.find_library('dtree'),
    ]
    extra_unit_files = [
        'service_files/set-spi-mux.service',
        'service_files/phal-reinit-devtree.service',
        'service_files/proc-pre-poweroff@.service',
        'service_files/op-clear-sys-dump-active@.service',
        'service_files/op-reset-host-check@.service',
        'service_files/op-reset-host-clear.service',
        'service_files/phal-import-devtree@.service',
        'service_files/phal-export-devtree@.service',
        'service_files/phal-create-boottime-guard-indicator.service',
        'service_files/op-clock-data-logger@.service',
    ]
    unit_subs.set('ENABLE_PHAL_TRUE', '')
endif

libgpiodcxx_dep = dependency('libgpiodcxx', default_options: ['bindings=cxx'])
sdbusplus_dep = dependency('sdbusplus')
sdeventplus_dep = dependency('sdeventplus')
pdi_dep = dependency('phosphor-dbus-interfaces')
phosphor_logging_dep = dependency('phosphor-logging')

gtest = dependency(
    'gtest',
    main: true,
    disabler: true,
    required: false,
    include_type: 'system',
)
if get_option('tests').allowed()
    if not gtest.found()
        gtest_proj = import('cmake').subproject('gtest', required: false)
        if gtest_proj.found()
            gtest = declare_dependency(
                dependencies: [
                    dependency('threads'),
                    gtest_proj.dependency('gtest'),
                    gtest_proj.dependency('gtest_main'),
                ],
            )
        else
            assert(
                not get_option('tests').enabled(),
                'Googletest is required if tests are enabled',
            )
        endif
    endif
endif

executable(
    'openpower-proc-control',
    [
        'cfam_access.cpp',
        'ext_interface.cpp',
        'filedescriptor.cpp',
        'proc_control.cpp',
        'targeting.cpp',
        'procedures/common/cfam_overrides.cpp',
        'procedures/common/cfam_reset.cpp',
        'procedures/common/collect_sbe_hb_data.cpp',
        'util.cpp',
    ] + extra_sources,
    dependencies: [
        libgpiodcxx_dep,
        cxx.find_library('pdbg'),
        pdi_dep,
        phosphor_logging_dep,
        sdbusplus_dep,
        dependency('threads'),
        dependency('fmt'),
    ] + extra_dependencies,
    install: true,
)

executable(
    'openpower-proc-nmi',
    ['nmi_main.cpp', 'nmi_interface.cpp'],
    dependencies: [
        cxx.find_library('pdbg'),
        pdi_dep,
        phosphor_logging_dep,
        sdbusplus_dep,
    ],
    install: true,
)

if build_phal
    executable(
        'phal-export-devtree',
        [
            'extensions/phal/devtree_export.cpp',
            'extensions/phal/fw_update_watch.cpp',
            'extensions/phal/pdbg_utils.cpp',
            'extensions/phal/create_pel.cpp',
            'util.cpp',
        ],
        dependencies: [
            phosphor_logging_dep,
            sdbusplus_dep,
            sdeventplus_dep,
            pdi_dep,
            cxx.find_library('pdbg'),
            cxx.find_library('phal'),
        ],
        install: true,
    )

    executable(
        'openpower-clock-data-logger',
        [
            'extensions/phal/clock_logger_main.cpp',
            'extensions/phal/clock_logger.cpp',
            'extensions/phal/create_pel.cpp',
            'util.cpp',
        ],
        dependencies: [
            cxx.find_library('dtree'),
            cxx.find_library('pdbg'),
            cxx.find_library('phal'),
            dependency('libdt-api'),
            phosphor_logging_dep,
            sdbusplus_dep,
            sdeventplus_dep,
        ],
        install: true,
    )
endif

unit_files = [
    'service_files/pcie-poweroff@.service',
    'service_files/xyz.openbmc_project.Control.Host.NMI.service',
    'service_files/op-stop-instructions@.service',
    'service_files/op-cfam-reset.service',
    'service_files/op-continue-mpreboot@.service',
    'service_files/op-enter-mpreboot@.service',
] + extra_unit_files

systemd_system_unit_dir = dependency('systemd').get_variable(
    'systemdsystemunitdir',
    pkgconfig_define: ['prefix', get_option('prefix')],
)
foreach u : unit_files
    configure_file(
        configuration: unit_subs,
        input: u + '.in',
        install: true,
        install_dir: systemd_system_unit_dir,
        output: '@BASENAME@',
    )
endforeach

if not get_option('tests').disabled()
    test(
        'utest',
        executable(
            'utest',
            'test/utest.cpp',
            'targeting.cpp',
            'filedescriptor.cpp',
            dependencies: [gtest, dependency('phosphor-logging')],
            implicit_include_directories: false,
            include_directories: '.',
        ),
    )
endif
