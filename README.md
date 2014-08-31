# constructor

## Building

Constructor is composed of C++ core which can be run on any platform. It relies on several cross-platform libraries such as SDL, Skia, libuv or V8. All of them are statically linked and compiled together with resource files into one large executable. The official build script downloads their recent copies & builds them. See `build.js` for details of how it is done.

Dependencies for building constructor include:

 - git
 - Node.js
 - GCC / Clang (C/C++ compiler)
 - Python2 (it has to be default python interpreter!)

To perform full build, run `node build.js`. You may pass additional argument to specify different target (android/iOS) or skip building dependencies (SDL/Skia/etc.).

## Directory structure

`/third_party/` - directory for dependencies
`/src/` - C++ sources
`/build/temp/` - temporary build results
`/build/results/` - build results
`/build_utils/` - utilities used by the build script