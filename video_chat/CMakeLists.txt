cmake_minimum_required(VERSION 3.10.0)
project(video_chat)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Conan configuration below
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(
    STATUS
      "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(
    DOWNLOAD
    "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
    "${CMAKE_BINARY_DIR}/conan.cmake" TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_configure(
  REQUIRES
  gtest/cci.20210126
  ffmpeg/4.4
  opencv/4.5.5
  tensorflow-lite/2.10.0
  zlib/1.2.13
  libpng/1.6.39
  libwebp/1.2.4
  eigen/3.4.0
  cpuinfo/cci.20220228
  imgui/1.89.1
  glfw/3.3.8
  glew/2.2.0
  IMPORTS
  "./res/bindings, imgui_impl_glfw* -> ./bindings"
  IMPORTS
  "./res/bindings, imgui_impl_opengl3* -> ./bindings"
  GENERATORS
  cmake_find_package)

set(CONAN_PROFILE "default")
conan_cmake_autodetect(settings)

conan_cmake_install(
  PATH_OR_REFERENCE
  .
  BUILD
  missing
  PROFILE
  ${CONAN_PROFILE}
  REMOTE
  conancenter
  SETTINGS
  ${settings}
  OPTIONS
  "opencv:with_ffmpeg=True;opencv:with_v4l=True")

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
# ftrapv checks for overflow and crashes the program if it happens.
add_compile_options(
  "$<$<CONFIG:DEBUG>:-O0;-g;-Wall;-Wextra;-pedantic;-Werror;-ftrapv;-fsanitize=undefined,address>"
)

add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=undefined,address>")

set(COMMON_INCLUDE "${CMAKE_SOURCE_DIR}/../common/include")
include_directories(include ${COMMON_INCLUDE} ${CMAKE_BINARY_DIR}/bindings)

enable_testing()

add_subdirectory(tests)
add_subdirectory(app)
add_subdirectory(src)
