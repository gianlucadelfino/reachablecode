cmake_minimum_required(VERSION 3.24.1)
project(simple_parser)

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "No CMAKE_BUILD_TYPE specified, using Release")
endif()

# Dependencies
find_package (Threads)

include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libraries)

include_directories(${LIB_DIR}/include)

enable_testing()
add_subdirectory(tests)
add_subdirectory(apps)
add_subdirectory(libraries)