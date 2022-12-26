cmake_minimum_required(VERSION 3.10.0)
project(almost_always_lockfree CXX)

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
  REQUIRES
  fmt/9.1.0
  REQUIRES
  tclap/1.2.5
  GENERATORS
  cmake_find_package)

conan_cmake_autodetect(settings)

conan_cmake_install(
  PATH_OR_REFERENCE
  .
  BUILD
  missing
  REMOTE
  conancenter
  SETTINGS
  ${settings})

# Dependencies
find_package(GTest REQUIRED)
find_package(fmt REQUIRED)
find_package(Threads)
find_package(tclap REQUIRED)

add_compile_options("$<$<CONFIG:RELEASE>:-Wall;-Wextra;-pedantic;-Werror>")
# ftrapv checks for overflow and crashes the program if it happens.
add_compile_options(
  "$<$<CONFIG:DEBUG>:-O0;-g;-Wall;-Wextra;-pedantic;-Werror;-ftrapv;-fsanitize=undefined,address>"
)

add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=undefined,address>")

include_directories(../common/include)

enable_testing()
add_subdirectory(tests)
