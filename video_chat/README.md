## Video Chat


# build

```
    mkdir build
    cd build
    CC=/usr/bin/clang-14 CXX=/usr/bin/clang++-14 conan install ../conanfile.txt  --profile:host=../emscripten.profile --profile=../default_clang.profile -r conancenter --build missing
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j
```

```
    CC=/usr/bin/clang-14 CXX=/usr/bin/clang++-14 conan install ../conanfile.txt  --profile:host=../emscripten.profile --profile:build=../default_clang.profile -r conancenter --build missing
```

```
    CC=/usr/bin/clang-14 CXX=/usr/bin/clang++-14 conan install ../conanfile.txt --profile=../default_clang.profile -r conancenter --build missing
```

```
 conan install ../conanfile.txt --profile:host=../emscripten.profile  --profile=../default_clang.profile -r conancenter --build missing
```

```
CC=/usr/bin/clang-14 CXX=/usr/bin/clang++-14 cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_WASM=OFF
```

CC=/home/gianluca/.conan/data/emsdk/3.1.17/_/_/package/2880313eadc30db92089af7733fe8364772ee5c8/bin/upstream/emscripten/emcc CXX=/home/gianluca/.conan/data/emsdk/3.1.17/_/_/package/2880313eadc30db92089af7733fe8364772ee5c8/bin/upstream/emscripten/em++ cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_WASM=ON


```
export PKG_CONFIG_PATH="/usr/lib/x86_64-linux-gnu/pkgconfig/:/usr/share/pkgconfig/"
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_WASM=ON -DCMAKE_TOOLCHAIN_FILE=/home/gianluca/.conan/data/emsdk/3.1.17/_/_/package/2880313eadc30db92089af7733fe8364772ee5c8/bin/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
```

CC=/home/gianluca/.conan/data/emsdk/3.1.17/_/_/package/2880313eadc30db92089af7733fe8364772ee5c8/bin/upstream/emscripten/emcc CXX=/home/gianluca/.conan/data/emsdk/3.1.17/_/_/package/2880313eadc30db92089af7733fe8364772ee5c8/bin/upstream/emscripten/em++ cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_WASM=ON -DCMAKE_TOOLCHAIN_FILE=/home/gianluca/.conan/data/emsdk/3.1.17/_/_/package/2880313eadc30db92089af7733fe8364772ee5c8/bin/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake

conan install ../conanfile.txt --profile:host=../emscripten.profile --profile:build=../default_clang.profile --build missing -o opencv:with_jpeg=False -o opencv:dnn=False -o opencv:with_tiff=False -o opencv:with_jpeg2000=False -o opencv:with_quirc=False -o ffmpeg:with_zlib=False -o ffmpeg:with_bzip2=False -o ffmpeg:with_lzma=False -o ffmpeg:with_libiconv=False -o ffmpeg:with_freetype=False -o ffmpeg:with_openjpeg=False -o ffmpeg:with_openh264=False -o ffmpeg:with_opus=False -o ffmpeg:with_vorbis=False -o ffmpeg:with_zeromq=False -o ffmpeg:with_sdl=False -o ffmpeg:with_libx264=False -o ffmpeg:with_libx265=False -o ffmpeg:with_libvpx=False -o ffmpeg:with_libmp3lame=False -o ffmpeg:with_libfdk_aac=False -o ffmpeg:with_libwebp=False -o ffmpeg:with_ssl=False -o ffmpeg:with_programs=False -o tensorflow-lite:with_xnnpack=False -o tensorflow-lite:with_ruy=False


CC=/usr/bin/emcc CXX=/usr/bin/em++ CXXFLAGS="-msimd128" CMAKE_TOOLCHAIN_FILE=/usr/share/emscripten/cmake/Modules/Platform/Emscripten.cmake conan install ../conanfile.txt --profile:host=../emscripten.profile --profile:build=../default_clang.profile --build missing -o opencv:with_jpeg=False -o opencv:dnn=False -o opencv:with_tiff=False -o opencv:with_jpeg2000=False -o opencv:with_quirc=False -o ffmpeg:with_zlib=False -o ffmpeg:with_bzip2=False -o ffmpeg:with_lzma=False -o ffmpeg:with_libiconv=False -o ffmpeg:with_freetype=False -o ffmpeg:with_openjpeg=False -o ffmpeg:with_openh264=False -o ffmpeg:with_opus=False -o ffmpeg:with_vorbis=False -o ffmpeg:with_zeromq=False -o ffmpeg:with_sdl=False -o ffmpeg:with_libx264=False -o ffmpeg:with_libx265=False -o ffmpeg:with_libvpx=False -o ffmpeg:with_libmp3lame=False -o ffmpeg:with_libfdk_aac=False -o ffmpeg:with_libwebp=False -o ffmpeg:with_ssl=False -o ffmpeg:with_programs=False -o tensorflow-lite:with_xnnpack=False -o tensorflow-lite:with_ruy=False
