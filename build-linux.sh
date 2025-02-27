#!/bin/bash

# Entferne die Verzeichnisse und die Datei, falls sie existieren
rm -rf ./node_modules/ ./build/ ./prebuilds/ ./package-lock.json

# Hol die aktuelle Version von Node.js
NODE_VERSION=$(node -v)
# Setze die Umgebungsvariablen
export NODE_VERSION="$NODE_VERSION"
export NODE_VERSION2="20.18.1"
export NODE_VERSION3="22.14.0"
export ELECTRON_VERSION="32.2.7"
export ELECTRON_VERSION2="34.3.0"

# Installiere die Abhängigkeiten
npm install 

# Führe prebuildify mit den gewünschten Optionen aus

npx prebuildify -t "$NODE_VERSION" -t "$NODE_VERSION2" -t "$NODE_VERSION3" -t "electron@$ELECTRON_VERSION" -t "electron@$ELECTRON_VERSION2" --napi=false --tag-uv --tag-armv --tag-libc --strip -n calculateNfp
npx prebuildify -t "$NODE_VERSION" -t "$NODE_VERSION2" -t "$NODE_VERSION3" -t "electron@$ELECTRON_VERSION" -t "electron@$ELECTRON_VERSION2" --napi=true --tag-uv --tag-armv --tag-libc --strip -n calculateNfp
# npx prebuildify -t "$NODE_VERSION" -t "electron@$ELECTRON_VERSION" --napi=false --tag-uv --tag-armv --tag-libc --strip --arch ia32
#npm install --arch=x64
#npx prebuildify -t "$NODE_VERSION" -t "$NODE_VERSION2" -t "$NODE_VERSION3" -t "electron@$ELECTRON_VERSION" -t "electron@$ELECTRON_VERSION2" --napi=false --tag-uv --tag-armv --tag-libc --strip -n calculateNfp --arch x64
#npx prebuildify -t "$NODE_VERSION" -t "$NODE_VERSION2" -t "$NODE_VERSION3" -t "electron@$ELECTRON_VERSION" -t "electron@$ELECTRON_VERSION2" --napi=true --tag-uv --tag-armv --tag-libc --strip -n calculateNfp --arch x64
#npm install --arch=arm64
#npx prebuildify -t "$NODE_VERSION" -t "$NODE_VERSION2" -t "$NODE_VERSION3" -t "electron@$ELECTRON_VERSION" -t "electron@$ELECTRON_VERSION2" --napi=false --tag-uv --tag-armv --tag-libc --strip -n calculateNfp --arch arm64
#npx prebuildify -t "$NODE_VERSION" -t "$NODE_VERSION2" -t "$NODE_VERSION3" -t "electron@$ELECTRON_VERSION" -t "electron@$ELECTRON_VERSION2" --napi=true --tag-uv --tag-armv --tag-libc --strip -n calculateNfp --arch arm64