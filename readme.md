# node-calculatenfp

This is one of the core libs of deepnest.

It will be build only for the version we currently need and support.



### Build on Windows:

You need the latest Version of `Visual Studio` at this moment 2022.
Install the Desktop C++ Environment and also enable all - at moment of writing - `v143` module (latest) ATL for x86, x84 and arm64
Then run `build-windows.ps1` in your powershell.

### Build on Linux:

Make sure you have installed nodejs, npm and basic build environment.
Then run `build-linux.sh`

Currently Supported: glibc (build on Ubuntu) and musl (build on Alpine) based hosts.

:info: We plan to cross-build once we need it. _hint: use linux with binfmt and use a node docker image and run it with platform arg..._

### Build on MacOS:

Make sure you have installed nodejs, npm and basic build environment.
Then run `build-macos.sh`