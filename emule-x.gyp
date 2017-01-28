{
    'targets': [
        {
            "variables": {
                "other_cflags%": "",
                "other_lflags%": "",
            },
            'target_name': 'emule_x',
            'type': 'shared_library',
            'include_dirs': [
                '/usr/local/include',
                '/usr/include',
                "..",
                "../boost-utils",
            ],
            'dependencies': [
                '../boost-utils/boost-utils.gyp:boost_utils',
            ],
            'sources': [
                'fs/fs.cpp',
                'fs/fs.hpp',
                'fs/sql.cpp',
                'ed2k/ed2k.cpp',
                'ed2k/ed2k.hpp',
                'encoding/encoding.cpp',
                'encoding/encoding.hpp',
                'protocol/ed2k_protocol.cpp',
                'protocol/ed2k_protocol.hpp',
                'protocol/opcodes.h',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'LD_DYLIB_INSTALL_NAME': "@rpath/$(EXECUTABLE_PATH)",
                        'MACOSX_DEPLOYMENT_TARGET': '10.11',
                        'OTHER_CFLAGS': [
                            "<(other_cflags)",
                        ],
                        'OTHER_LDFLAGS': [
                            "-stdlib=libc++",
                            "-L/usr/local/lib",
                            "-L../boost-utils/build/Default",
                            '-lboost_system',
                            "-lboost_iostreams",
                            "-lboost_filesystem",
                            "-lboost_thread",
                            "-lboost_utils",
                            "-lz",
                            "-lcrypto",
                            "-lsqlite3",
                            "-lboost_regex",
                            "-Wl,-rpath,.",
                            "<(other_cflags)",
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
                "..",
                "../boost-utils",
            ],
            'sources': [
                'fs/fs_test.hpp',
                'encoding/encoding_test.hpp',
                'ed2k/ed2k_test.hpp',
                'protocol/ed2k_protocol_test.hpp',
                'all_test.cpp',
            ],
            'dependencies': [
                'emule_x',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.11',
                        'OTHER_LDFLAGS': [
                            "-stdlib=libc++",
                            "-L.",
                            "-L/usr/local/lib",
                            "-L../boost-utils/build/Default",
                            '-lboost_system',
                            "-lboost_iostreams",
                            "-lboost_unit_test_framework",
                            "-lboost_utils",
                            "-lboost_thread",
                            "-lboost_filesystem",
                            "-lemule_x",
                            "-lz",
                            "-lcrypto",
                            "-lsqlite3",
                            "-lboost_regex",
                            "-Wl,-rpath,$(PROJECT_DIR)/../boost-utils/build/Default",
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
                "..",
                "../boost-utils",
            ],
            'dependencies': [
                'emule_x',
            ],
            'sources': [
                'fs/fs_test.hpp',
                'ed2k/console_test.cpp',
                'test/console_test.cpp',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.11',
                        'OTHER_LDFLAGS': [
                            "-stdlib=libc++",
                            "-L.",
                            "-L/usr/local/lib",
                            "-L../boost-utils/build/Default",
                            '-lboost_system',
                            "-lboost_iostreams",
                            "-lboost_unit_test_framework",
                            "-lboost_utils",
                            "-lemule_x",
                            "-lz",
                            "-lcrypto",
                            "-lsqlite3",
                            "-lboost_regex",
                            "-Wl,-rpath,$(PROJECT_DIR)/../boost-utils/build/Default",
                            "-Wl,-rpath,."
                        ],
                    },
                }],
            ],
        },
    ],
}
