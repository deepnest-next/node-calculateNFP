# Building @deepnest/calculate-nfp

A C++ Node addon built with node-gyp and packaged with prebuildify (`build-*.sh` / `.ps1`).
Boost.Polygon is vendored in `src/`, so no Boost install is needed.

The npm package contains only `index.js` and the N-API prebuilds (`files` in `package.json`), no sources and no install script. One N-API binary per platform works for every Node and Electron version. A platform without a prebuild (e.g. musl, FreeBSD) gets "No native build was found" at runtime and has to build from this repository.

## External dependencies

| Needed for | Debian/Ubuntu | Fedora | Alpine (musl) | macOS | Windows |
|---|---|---|---|---|---|
| Node.js + npm (the scripts build for the running Node plus fixed Node/Electron versions) | `nodejs npm` (or nvm) | `nodejs npm` | `nodejs npm` | `brew install node` | Node.js installer |
| node-gyp: Python 3 | `python3` | `python3` | `python3` | preinstalled | Python 3 from python.org |
| node-gyp: C++ toolchain + make | `build-essential` | `gcc-c++ make` | `build-base` | Xcode Command Line Tools | Visual Studio 2022, "Desktop development with C++", v143 build tools + ATL for x86/x64/arm64 (`build-windows-install.ps1` locates or installs the Visual Studio Installer) |
| `rust-minkowski/` (separate Rust crate, not part of the npm build) | Rust toolchain + the C++ toolchain above | same | same | same | same |

prebuildify downloads the Node and Electron headers for every target version, so the build
needs network access.

## Build

```bash
./build-linux.sh        # Linux (glibc or musl)
./build-macos.sh        # macOS
./build-windows.ps1     # Windows (PowerShell)
npm test
```

```bash
cd rust-minkowski && cargo build && cargo test
```
