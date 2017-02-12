{
    "variables": {
        "other_cflags%": "",
        "other_lflags%": "",
        "deps%": [
            '-lboost_system',
            "-lboost_iostreams",
            "-lboost_filesystem",
            "-lboost_thread-mt",
            "-lboost_chrono-mt",
            "-lboost_utils",
            "-lcurl",
            "-ljson-c",
            "-lz",
            "-lcrypto",
            "-lsqlite3",
            "-lboost_regex",
        ],
    },
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
                'runner/runner.cpp',
                'runner/runner.hpp',
                'fs/fs.cpp',
                'fs/fs.hpp',
                'fs/sql.cpp',
                'ed2k/ed2k.cpp',
                'ed2k/ed2k.hpp',
                'encoding/encoding.cpp',
                'encoding/encoding.hpp',
                'protocol/ed2k_protocol.cpp',
                'protocol/ed2k_protocol.hpp',
                'protocol/kadx_protocol.cpp',
                'protocol/kadx_protocol.hpp',
                'protocol/opcodes.h',
                'kadx/kadx.cpp',
                'kadx/kadx.hpp',
                'ws/ws.cpp',
                'ws/ws.hpp',
                'ws/curl_util.h',
                'ws/curl_util.c',
            ],
            "ldflags":[
                "<(deps)",
            ],
            'cflags': [
                '-std=c++11',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.11',
                        'LD_DYLIB_INSTALL_NAME': "@rpath/$(EXECUTABLE_PATH)",
                        'OTHER_CFLAGS': [
                            "<(other_cflags)",
                        ],
                        'OTHER_LDFLAGS': [
                            "-stdlib=libc++",
                            "-L/usr/local/lib",
                            "-L.",
                            "-L../boost-utils/build/Default",
                            "-Wl,-rpath,.",
                            "<(other_lflags)",
                            "<(deps)",
                        ],
                    },
                }],
                ['OS=="linux"', {
                    'ldflags': [
                        '-pthread',
                        "-L.",
                        "-L/usr/local/lib",
                        "-Wl,-rpath,.",
                    ],
                    'cflags': [
                        '-fPIC',
                    ],
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
                'runner/runner_test.hpp',
                'fs/fs_test.hpp',
                'encoding/encoding_test.hpp',
                'ed2k/ed2k_test.hpp',
                'protocol/ed2k_protocol_test.hpp',
                'all_test.cpp',
            ],
            'dependencies': [
                'emule_x',
            ],
            "ldflags":[
                "<(deps)",
            ],
            'cflags': [
                '-std=c++11',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.11',
                        'LD_DYLIB_INSTALL_NAME': "@rpath/$(EXECUTABLE_PATH)",
                        'OTHER_CFLAGS': [
                            "<(other_cflags)",
                        ],
                        'OTHER_LDFLAGS': [
                            "-stdlib=libc++",
                            "-L/usr/local/lib",
                            "-L.",
                            "-L../boost-utils/build/Default",
                            "-Wl,-rpath,.",
                            "<(other_lflags)",
                            "<(deps)",
                        ],
                    },
                }],
                ['OS=="linux"', {
                    'ldflags': [
                        '-pthread',
                        "-L.",
                        "-L/usr/local/lib",
                        "-Wl,-rpath,.",
                    ],
                    'cflags': [
                        '-fPIC',
                    ],
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
                'ws/ws_test_c.cpp',
                'runner/runner_test_c.cpp',
                'fs/fs_test.hpp',
                'ed2k/console_test.cpp',
                'test/console_test.cpp',
            ],
            "ldflags":[
                "<(deps)",
            ],
            'cflags': [
                '-std=c++11',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.11',
                        'LD_DYLIB_INSTALL_NAME': "@rpath/$(EXECUTABLE_PATH)",
                        'OTHER_CFLAGS': [
                            "<(other_cflags)",
                        ],
                        'OTHER_LDFLAGS': [
                            "-stdlib=libc++",
                            "-L/usr/local/lib",
                            "-L.",
                            "-L../boost-utils/build/Default",
                            "-Wl,-rpath,.",
                            "<(other_lflags)",
                            "<(deps)",
                        ],
                    },
                }],
                ['OS=="linux"', {
                    'ldflags': [
                        '-pthread',
                        "-L.",
                        "-L/usr/local/lib",
                        "-Wl,-rpath,.",
                    ],
                    'cflags': [
                        '-fPIC',
                    ],
                }],
            ],
        },
    ],
}
