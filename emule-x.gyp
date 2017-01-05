{
    'targets': [
        {
            'target_name': 'emule_x',
            'type': 'shared_library',
            'include_dirs': [
                '/usr/local/include',
                '/usr/include',
                "./libs/"
            ],
            'dependencies': [
                './libs/boost-utils/boost-utils.gyp:boost_utils',
            ],
            'sources': [
                'ed2k/ed2k.cpp',
                'ed2k/ed2k.hpp',
                'protocol/encoding.cpp',
                'protocol/encoding.hpp',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'LD_DYLIB_INSTALL_NAME': "@rpath/$(EXECUTABLE_PATH)",
                        'MACOSX_DEPLOYMENT_TARGET': '10.5',
                        'OTHER_LDFLAGS': [
                            "-L/usr/local/lib",
                            "-L./libs/boost-utils/build/Default",
                            '-lboost_system',
                            "-lboost_iostreams",
                            "-lboost_filesystem",
                            "-lboost_thread",
                            "-lboost_utils",
                            "-Wl,-rpath,.",
                        ],
                    },
                }],
            ],
        },
        {
            'target_name': 'emule_x.test',
            'type': 'executable',
            'include_dirs': [
                '/usr/local/include',
                '/usr/include',
                "./libs/"
            ],
            'sources': [
                'ed2k/ed2k_test.hpp',
                'protocol/encoding_test.hpp',
                'all_test.cpp',
            ],
            'dependencies': [
                'emule_x',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.5',
                        'OTHER_LDFLAGS': [
                            "-L.",
                            "-L/usr/local/lib",
                            "-L./libs/boost-utils/build/Default",
                            '-lboost_system',
                            "-lboost_iostreams",
                            "-lboost_unit_test_framework",
                            "-lboost_utils",
                            "-lemule_x",
                            "-Wl,-rpath,$(PROJECT_DIR)/libs/boost-utils/build/Default",
                            "-Wl,-rpath,."
                        ],
                    },
                }],
            ],
        },
        {
            'target_name': 'emule_x.test.console',
            'type': 'executable',
            'include_dirs': [
                '/usr/local/include',
                '/usr/include',
                "./libs/"
            ],
            'dependencies': [
                'emule_x',
            ],
            'sources': [
                'ed2k/console_test.cpp',
                'test/console_test.cpp',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.5',
                        'OTHER_LDFLAGS': [
                            "-L.",
                            "-L/usr/local/lib",
                            "-L./libs/boost-utils/build/Default",
                            '-lboost_system',
                            "-lboost_iostreams",
                            "-lboost_unit_test_framework",
                            "-lboost_utils",
                            "-lemule_x",
                            "-Wl,-rpath,$(PROJECT_DIR)/libs/boost-utils/build/Default"
                        ],
                    },
                }],
            ],
        },
    ],
}
