cmake_minimum_required(VERSION 3.0)

project(MonteCarloOpenCL)

# This will display the full g++ command in the output.
set(CMAKE_VERBOSE_MAKEFILE ON)

# Dipendencies
find_package(PkgConfig REQUIRED)

# pthread
find_package (Threads)

# OpenCL
find_package(OpenCL REQUIRED)

# set the 3rd parties include
set(3RD_PARTIES_INCLUDE "../3rdParties")

# Set this to the location of RandomCL generators
add_compile_definitions(GENERATOR_LOCATION="${CMAKE_SOURCE_DIR}/../3rdParties/RandomCL/generators")

set(GCC_WARNING_FLAGS "-Wall -Werror -Wextra -pedantic")

# ftrapv checks for overflow and crashes the program if it happens.
#set(GCC_DEBUG_FLAGS "-O0 -g -ftrapv")
#set(GCC_SANITIZE_FLAGS "-fsanitize=undefined,address")
#set(LSAN_OPTIONS "verbosity=1:log_threads=1")

# This needs at least gcc9 for this
SET(CMAKE_CXX_COMPILER /usr/bin/g++)
set(GCC_DIALECT_FLAGS "-std=c++17")

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

configure_file(src/calculate_trajectory.cl calculate_trajectory.cl COPYONLY)
configure_file(create_histogram.gnuplot create_histogram.gnuplot COPYONLY)

add_executable(MonteCarloOpenCL ${SRCS} ${HDRS})
target_link_libraries(MonteCarloOpenCL ${OpenCL_LIBRARY})
target_include_directories(MonteCarloOpenCL PUBLIC include ${OpenCL_INCLUDE_DIRS})
include_directories(SYSTEM ${3RD_PARTIES_INCLUDE})