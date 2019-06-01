cmake_minimum_required(VERSION 3.0)

project(allocCatcher)

# This will display the full g++ command in the output.
set(CMAKE_VERBOSE_MAKEFILE ON)

set(GCC_WARNING_FLAGS "-Wall -Werror -Wextra -pedantic")

# ftrapv checks for overflow and crashes the program if it happens.
set(GCC_DEBUG_FLAGS "-g -ftrapv")
set(GCC_SANITIZE_FLAGS "-fsanitize=undefined,address")

# This needs at least gcc7 for this
set(GCC_DIALECT_FLAGS "-std=c++17")

# Change this for release
set(CMAKE_BUILD_TYPE "debug")

if(CMAKE_BUILD_TYPE MATCHES "[Rr][Ee][Ll][Ee][Aa][Ss][Ee]")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS}")
elseif(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]" OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS} ${GCC_SANITIZE_FLAGS} ${GCC_DEBUG_FLAGS}")
endif()

add_executable(allocCatcher src/main.cpp)
