cmake_minimum_required(VERSION 3.0)

project(exshelf)

# This will display the full g++ command in the output.
set(CMAKE_VERBOSE_MAKEFILE ON)

# Dipendencies
find_package(PkgConfig REQUIRED)

# pthread
find_package(Threads)

# OpenCv
find_package(OpenCV 4 REQUIRED)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}")
# Tesseract
find_package(Tesseract REQUIRED)

# set the 3rd parties include
set(3RD_PARTIES_INCLUDE "${CMAKE_SOURCE_DIR}/../3rdParties")

set(LOCAL_INCLUDE "include")

# set the common include
set(COMMON_INCLUDE "${CMAKE_SOURCE_DIR}/../common/include")

set(GCC_WARNING_FLAGS "-Wall -Werror -Wextra -pedantic")

# ftrapv checks for overflow and crashes the program if it happens.
set(GCC_DEBUG_FLAGS "-O0 -g -ftrapv")

#set(GCC_SANITIZE_FLAGS "-fsanitize=undefined,address")
#set(LSAN_OPTIONS "verbosity=1:log_threads=1")

# We use clang to be able to backlist some sanitizers errors from
# libfontconfig
set(CMAKE_CXX_COMPILER /usr/bin/clang)
set(CLANG_LINK_LIBS "stdc++" "ubsan")
set(MATH_LIBS "m")
set (CMAKE_CXX_STANDARD 17)

# Change this for release
set(CMAKE_BUILD_TYPE "debug")

if(CMAKE_BUILD_TYPE MATCHES "[Rr][Ee][Ll][Ee][Aa][Ss][Ee]")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS}")
elseif(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]" OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS} ${GCC_SANITIZE_FLAGS} ${GCC_DEBUG_FLAGS}")
elseif(CMAKE_BUILD_TYPE MATCHES "[Pp][Rr][Oo][Ff][Ii][Ll][Ee]")
    set(CMAKE_CXX_FLAGS "-pg ${CMAKE_CXX_FLAGS} -O1 -DNDEBUG ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS}")
endif()

configure_file(res/shelfTest2_small.jpg shelfTest2_small.jpg COPYONLY)

set(SRCS src/main.cpp)

add_executable(exshelf ${SRCS} ${HDRS})

target_link_libraries(exshelf ${OpenCV_LIBS} ${CLANG_LINK_LIBS} ${TESSERACT_LIBRARIES} ${MATH_LIBS} ${CMAKE_THREAD_LIBS_INIT})

include_directories(SYSTEM ${LOCAL_INCLUDE} ${3RD_PARTIES_INCLUDE} ${TESSERACT_INCLUDE_DIR} ${COMMON_INCLUDE})
