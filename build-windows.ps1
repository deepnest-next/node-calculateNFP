whoami
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ./build-windows-install.ps1

Remove-Item -ErrorAction SilentlyContinue -Recurse -Force .\node_modules\, .\build\, .\prebuilds\, .\package-lock.json

# Hol die aktuelle Version von Node.js
$NodeVersion = &node -v
# Setze sie in eine Umgebungsvariable
[System.Environment]::SetEnvironmentVariable('NODE_VERSION', $NodeVersion, [System.EnvironmentVariableTarget]::Process)
[System.Environment]::SetEnvironmentVariable('NODE_VERSION2', "20.18.1", [System.EnvironmentVariableTarget]::Process)
[System.Environment]::SetEnvironmentVariable('NODE_VERSION3', "22.14.0", [System.EnvironmentVariableTarget]::Process)
[System.Environment]::SetEnvironmentVariable('ELECTRON_VERSION', "32.3.1", [System.EnvironmentVariableTarget]::Process)
[System.Environment]::SetEnvironmentVariable('ELECTRON_VERSION2', "34.3.0", [System.EnvironmentVariableTarget]::Process)

npm install --arch=ia32

npx prebuildify -t $env:NODE_VERSION -t $env:NODE_VERSION2 -t $env:NODE_VERSION3 -t electron@$env:ELECTRON_VERSION -t electron@$env:ELECTRON_VERSION2 --napi=false --tag-libc --strip -n calculateNfp --arch ia32
npx prebuildify -t $env:NODE_VERSION -t $env:NODE_VERSION2 -t $env:NODE_VERSION3 -t electron@$env:ELECTRON_VERSION -t electron@$env:ELECTRON_VERSION2 --napi=true --tag-libc --strip -n calculateNfp --arch ia32

npm install --arch=x64
npx prebuildify -t $env:NODE_VERSION -t $env:NODE_VERSION2 -t $env:NODE_VERSION3 -t electron@$env:ELECTRON_VERSION -t electron@$env:ELECTRON_VERSION2 --napi=false --tag-libc --strip -n calculateNfp --arch x64
npx prebuildify -t $env:NODE_VERSION -t $env:NODE_VERSION2 -t $env:NODE_VERSION3 -t electron@$env:ELECTRON_VERSION -t electron@$env:ELECTRON_VERSION2 --napi=true --tag-libc --strip -n calculateNfp --arch x64

npm install --arch=arm64
npx prebuildify -t $env:NODE_VERSION -t $env:NODE_VERSION2 -t $env:NODE_VERSION3 -t electron@$env:ELECTRON_VERSION -t electron@$env:ELECTRON_VERSION2 --napi=false --tag-libc --strip -n calculateNfp --arch arm64
npx prebuildify -t $env:NODE_VERSION -t $env:NODE_VERSION2 -t $env:NODE_VERSION3 -t electron@$env:ELECTRON_VERSION -t electron@$env:ELECTRON_VERSION2 --napi=true --tag-libc --strip -n calculateNfp --arch arm64
