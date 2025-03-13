{
  "variables": {
    "openssl_fips": ""
  },
  "targets": [
    {
      "target_name": "addon",
      "sources": ["src/addon.cc", "src/minkowski.cc"],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
      "conditions": [
        [
          'OS=="win"', {
            "msvs_version": 2022,
            
            "conditions": [
              [
                'target_arch=="ia32"', {
                  "cflags!": ["-fno-exceptions", "-m32"],
                  "ldflags": ["-m elf_i386"],
                  "defines": ["NOMINMAX"]
                }
              ],
              [
                'target_arch=="x64"', {
                  "cflags!": ["-fno-exceptions", "-m64"],
                  "ldflags": ["-m elf_x86_64"],
                  "defines": ["NOMINMAX"],
                }
              ],
              [
                'target_arch=="arm64"', {
                  "cflags!": ["-fno-exceptions"],
                  "ldflags": ["-m elf_arm64"],
                  "defines": ["NOMINMAX"]
                }
              ]
            ]
          }
        ],
        [
          'OS=="mac"', {
            "xcode_settings": {
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
            }
          }
        ]
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "./src/polygon/include"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ]
    }
  ]
}
