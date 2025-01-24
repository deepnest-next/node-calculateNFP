Remove-Item -ErrorAction SilentlyContinue -Recurse -Force .\node_modules\, .\build\, .\prebuilds\, .\package-lock.json
npm install
node-gyp rebuild