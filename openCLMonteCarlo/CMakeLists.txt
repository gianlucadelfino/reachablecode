cmake_minimum_required(VERSION 3.24.1)

project(MonteCarloOpenCL)

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

# set the 3rd parties include
set(3RD_PARTIES_INCLUDE "../3rdParties")

# Set this to the location of RandomCL generators
add_compile_definitions(GENERATOR_LOCATION="${CMAKE_SOURCE_DIR}/../3rdParties/RandomCL/generators")

add_compile_options("$<$<CONFIG:RELEASE>:-Wall;-Wextra;-pedantic;-Werror>")
# ftrapv checks for overflow and crashes the program if it happens.
add_compile_options("$<$<CONFIG:DEBUG>:-O0;-g;-Wall;-Wextra;-pedantic;-Werror;-ftrapv;>")

set(SRCS src/main.cpp)

configure_file(src/calculate_trajectory.cl calculate_trajectory.cl COPYONLY)
configure_file(create_histogram.gnuplot create_histogram.gnuplot COPYONLY)

add_executable(MonteCarloOpenCL ${SRCS})
target_include_directories(MonteCarloOpenCL PUBLIC include)
target_link_libraries(MonteCarloOpenCL PRIVATE OpenCL)
include_directories(SYSTEM ${3RD_PARTIES_INCLUDE})