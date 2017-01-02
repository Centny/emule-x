{
    'targets': [
        {
            'target_name': 'ed2k',
            'type': 'shared_library',
            'include_dirs': [
                '/usr/local/include',
                '/usr/include',
                "../libs/"
            ],
            'dependencies': [
                '../libs/boost-utils/boost-utils.gyp:boost-utils',
            ],
            'sources': [
                'ed2k.cpp',
                'ed2k.hpp',
                'encoding.cpp',
                'encoding.hpp',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.1',
                        'OTHER_LDFLAGS': [
                            "-L/usr/local/lib",
                            '-lboost_system',
                            "-lboost_iostreams",
                            "-lboost_filesystem",
                            "-lboost_thread",
                            "-Wall",
                        ],
                    },
                }],
            ],
        },
        {
            'target_name': 'ed2k_test',
            'type': 'executable',
            'include_dirs': [
                '/usr/local/include',
                '/usr/include',
                "../libs/"
            ],
            'sources': [
                'ed2k.cpp',
                'ed2k.hpp',
                'ed2k_test.hpp',
                'encoding.cpp',
                'encoding.hpp',
                'encoding_test.hpp',
                'all_test.cpp',
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.1',
                        'OTHER_LDFLAGS': [
                            "-L/usr/local/lib",
                            '-lboost_system',
                            "-lboost_iostreams",
                            "-lboost_unit_test_framework",
                        ],
                    },
                }],
            ],
        },
    ],
}
