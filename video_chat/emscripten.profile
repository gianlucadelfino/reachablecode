[settings]
os=Emscripten
arch=wasm
compiler=clang
compiler.version=14
build_type=Release

#[build_requires]
#[tool_requires]
#cmake/3.19.8
#emsdk/3.1.23
[env]
CC=/usr/bin/emcc
CXX=/usr/bin/em++
CMAKE_TOOLCHAIN_FILE=/usr/share/emscripten/cmake/Modules/Platform/Emscripten.cmake
CONAN_CMAKE_PROGRAM=
CV_SIMD=0