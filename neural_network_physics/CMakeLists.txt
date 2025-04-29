cmake_minimum_required(VERSION 3.24.1)
project(neural_network_physics)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# pthread
find_package (Threads)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "No CMAKE_BUILD_TYPE specified, using Release")
endif()

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

add_compile_options("$<$<CONFIG:RELEASE>:-Wall;-Wextra;-pedantic;-Werror>")
# ftrapv checks for overflow and crashes the program if it happens.
add_compile_options("$<$<CONFIG:DEBUG>:-O0;-g;-Wall;-Wextra;-pedantic;-Werror;-ftrapv;-fsanitize=undefined,address>")

add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=undefined,address>")

set(COMMON_INCLUDE "${CMAKE_SOURCE_DIR}/../common/include")
include_directories(include ${COMMON_INCLUDE})

enable_testing()

add_subdirectory(tests)
add_subdirectory(example)
add_subdirectory(src)
