{
    'variables': {
        "library": "shared_library",
    },
    'includes': [
    ],
    'targets': [
        {
            'target_name': 'emule-x',
            'type': '<(library)',
            'defines': [
            ],
            'dependencies': [
                'ed2k/ed2k.gyp:ed2k',
            ],
            'include_dirs': [
                '/usr/local/include',
                '/usr/include',
            ],
            'sources': [
            ],
        },
    ],
}
