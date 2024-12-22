#!/bin/bash

find ./src -type f -print0 | sort -z | xargs -0 sha256sum > current-shasum.txt
sha256sum ./build-* ./binding.gyp >> current-shasum.txt || true
echo "ZZZZ:ENV:Node:${NODE_VERSION},Electron:${ELECTRON_VERSION}" >> current-shasum.txt

sort current-shasum.txt -o current-shasum.txt

# Generate the final SHASUM for comparison
sha256sum current-shasum.txt | awk '{print $1}' > sha256.txt