#!/bin/bash

# Entferne die Verzeichnisse und die Datei, falls sie existieren
rm -rf ./node_modules/ ./build/ ./prebuilds/ ./package-lock.json

# Hol die aktuelle Version von Node.js
NODE_VERSION=$(node -v)
# Setze die Umgebungsvariablen
export NODE_VERSION="$NODE_VERSION"
export NODE_VERSION="20.18.1"
export ELECTRON_VERSION="32.2.7"

# Installiere die Abhängigkeiten
npm install

# Führe prebuildify mit den gewünschten Optionen aus
# npx prebuildify -t "$NODE_VERSION" -t "electron@$ELECTRON_VERSION" --napi=false --tag-uv --tag-armv --tag-libc --strip --arch ia32
npx prebuildify -t "$NODE_VERSION" -t "electron@$ELECTRON_VERSION" --napi=false --tag-uv --tag-armv --tag-libc --strip --arch x64
npx prebuildify -t "$NODE_VERSION" -t "electron@$ELECTRON_VERSION" --napi=false --tag-uv --tag-armv --tag-libc --strip --arch arm64