cmake_minimum_required(VERSION 3.10.0)
project(video_chat)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CONAN_DISABLE_CHECK_COMPILER
    True
    CACHE BOOL "" FORCE)

# set(CMAKE_CXX_STANDARD 20) set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_WASM "Build for webassembly" OFF)

# Conan configuration
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(
    STATUS
      "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(
    DOWNLOAD
    "https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake"
    "${CMAKE_BINARY_DIR}/conan.cmake" TLS_VERIFY ON)
endif()

# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake) conan_basic_setup()

include(${CMAKE_BINARY_DIR}/conan.cmake)

set(CONAN_OPTIONS "opencv:with_ffmpeg=True")

# Emscripten configuration
if(BUILD_WASM)
  set(CONAN_PROFILE_HOST "${CMAKE_SOURCE_DIR}/emscripten.profile")
else()
  set(CONAN_PROFILE_HOST "${CMAKE_SOURCE_DIR}/default_clang.profile")
  list(APPEND CONAN_OPTIONS "opencv:with_v4l=True")
endif()

list(APPEND CONAN_OPTIONS "opencv:with_jpeg=False")
list(APPEND CONAN_OPTIONS "opencv:dnn=False")
list(APPEND CONAN_OPTIONS "opencv:with_tiff=False")
list(APPEND CONAN_OPTIONS "opencv:with_jpeg2000=False")
list(APPEND CONAN_OPTIONS "opencv:with_quirc=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_zlib=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_bzip2=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_lzma=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_libiconv=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_freetype=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_openjpeg=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_openh264=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_opus=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_vorbis=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_zeromq=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_sdl=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_libx264=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_libx265=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_libvpx=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_libmp3lame=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_libfdk_aac=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_libwebp=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_ssl=False")
list(APPEND CONAN_OPTIONS "ffmpeg:with_programs=False")
list(APPEND CONAN_OPTIONS "tensorflow-lite:with_xnnpack=False")
list(APPEND CONAN_OPTIONS "tensorflow-lite:with_ruy=False")

conan_cmake_configure(
  REQUIRES
  gtest/cci.20210126
  ffmpeg/4.4
  opencv/4.5.5
  tensorflow-lite/2.10.0
  eigen/3.4.0
  imgui/1.89.1
  #glfw/3.3.8
  glew/2.2.0
  IMPORTS
  "./res/bindings, imgui_impl_glfw* -> ./bindings"
  IMPORTS
  "./res/bindings, imgui_impl_opengl3* -> ./bindings"
  GENERATORS
  cmake_find_package
  CMakeToolchain)

# set(CONAN_PROFILE_BUILD "default")
set(CONAN_PROFILE_BUILD "${CMAKE_SOURCE_DIR}/default_clang.profile")

# conan_cmake_autodetect(settings)

message("CMAKE_CXX_COMPILER_ID ${CMAKE_CXX_COMPILER_ID}")
message("\nsettings " ${settings})

conan_cmake_install(
  PATH_OR_REFERENCE
  .
  BUILD
  missing
  PROFILE_BUILD
  ${CONAN_PROFILE_BUILD}
  PROFILE_HOST
  ${CONAN_PROFILE_HOST}
  REMOTE
  conancenter
  SETTINGS
  ${settings}
  OPTIONS
  ${CONAN_OPTIONS})

# Dependencies
find_package(GTest REQUIRED)
find_package(ffmpeg REQUIRED)
find_package(OpenCV REQUIRED)
find_package(tensorflowlite REQUIRED)
find_package(imgui REQUIRED)
find_package(Threads REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE Release)
  message(STATUS "No CMAKE_BUILD_TYPE specified, using Release")
endif()

add_compile_options("$<$<CONFIG:RELEASE>:-Wall;-Wextra;-pedantic;-Werror>")

if(NOT BUILD_WASM)
  # ftrapv checks for overflow and crashes the program if it happens.
  add_compile_options(
    "$<$<CONFIG:DEBUG>:-O0;-g;-Wall;-Wextra;-pedantic;-Werror;-ftrapv;-fsanitize=undefined,address>"
  )
  add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=undefined,address>")
else()
  add_compile_options(
    "$<$<CONFIG:DEBUG>:-O0;-g;-Wall;-Wextra;-pedantic;-Werror;>")
endif()

set(COMMON_INCLUDE "${CMAKE_SOURCE_DIR}/../common/include")
include_directories(include ${COMMON_INCLUDE} ${CMAKE_BINARY_DIR}/bindings)

# enable_testing()

# add_subdirectory(tests)
add_subdirectory(app)
add_subdirectory(src)
