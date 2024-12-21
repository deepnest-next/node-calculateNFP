Remove-Item -ErrorAction SilentlyContinue -Recurse -Force .\node_modules\, .\build\, .\prebuilds\, .\package-lock.json

# Hol die aktuelle Version von Node.js
$NodeVersion = &node -v
# Setze sie in eine Umgebungsvariable
[System.Environment]::SetEnvironmentVariable('NODE_VERSION', $NodeVersion, [System.EnvironmentVariableTarget]::Process)
[System.Environment]::SetEnvironmentVariable('NODE_VERSION', "20.18.1", [System.EnvironmentVariableTarget]::Process)
[System.Environment]::SetEnvironmentVariable('ELECTRON_VERSION', "32.2.7", [System.EnvironmentVariableTarget]::Process)

npm install

npx prebuildify -t $env:NODE_VERSION -t electron@$env:ELECTRON_VERSION --napi=false --tag-uv --tag-armv --tag-libc --strip --arch ia32
npx prebuildify -t $env:NODE_VERSION -t electron@$env:ELECTRON_VERSION --napi=false --tag-uv --tag-armv --tag-libc --strip --arch x64
#npx prebuildify -t $env:NODE_VERSION -t electron@$env:ELECTRON_VERSION --napi=false --tag-uv --tag-armv --tag-libc --strip --arch arm64