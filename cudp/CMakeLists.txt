cmake_minimum_required(VERSION 3.0)

project(cUDP)

# This will display the full g++ command in the output.
set(CMAKE_VERBOSE_MAKEFILE ON)

# Dipendencies
find_package(PkgConfig REQUIRED)

# pthread
find_package(Threads)

# OpenCv
find_package(OpenCV REQUIRED)

# set the 3rd parties include
set(3RD_PARTIES_INCLUDE "../3rdParties")

set(GCC_WARNING_FLAGS "-Wall -Werror -Wextra -pedantic")

# ftrapv checks for overflow and crashes the program if it happens.
set(GCC_DEBUG_FLAGS "-O0 -g -ftrapv")

# set the common include
set(COMMON_INCLUDE "${CMAKE_SOURCE_DIR}/../common/include")

# NB Blacklist for libfontconfig
configure_file(sanitizer_blacklist.txt sanitizer_blacklist.txt COPYONLY)

set(GCC_SANITIZE_FLAGS "-fsanitize=undefined,address")
set(LSAN_OPTIONS "verbosity=1:log_threads=1")

# We use clang to be able to backlist some sanitizers errors from
# libfontconfig
set(CMAKE_CXX_COMPILER /usr/bin/clang)
set(GCC_DIALECT_FLAGS "-std=c++17")
seT(CLANG_LINK_LIBS "stdc++" "ubsan")

# Change this for release
set(CMAKE_BUILD_TYPE "debug")

if(CMAKE_BUILD_TYPE MATCHES "[Rr][Ee][Ll][Ee][Aa][Ss][Ee]")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS}")
elseif(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]" OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS} ${GCC_SANITIZE_FLAGS} ${GCC_DEBUG_FLAGS}")
elseif(CMAKE_BUILD_TYPE MATCHES "[Pp][Rr][Oo][Ff][Ii][Ll][Ee]")
    set(CMAKE_CXX_FLAGS "-pg ${CMAKE_CXX_FLAGS} -O1 -DNDEBUG ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS}")
endif()

set(SRCS src/main.cpp)

add_executable(cUDP ${SRCS} ${HDRS})

target_link_libraries(cUDP ${OpenCV_LIBS} ${CLANG_LINK_LIBS})

include_directories(SYSTEM ${3RD_PARTIES_INCLUDE} ${COMMON_INCLUDE})