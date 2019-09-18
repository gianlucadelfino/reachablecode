cmake_minimum_required(VERSION 3.0)

project(ASCII-UML-Generator)

# This will display the full g++ command in the output.
set(CMAKE_VERBOSE_MAKEFILE ON)

set(GCC_WARNING_FLAGS "-Wall -Werror -Wextra -pedantic")

# ftrapv checks for overflow and crashes the program if it happens.
set(GCC_DEBUG_FLAGS "-g -ftrapv")
set(GCC_SANITIZE_FLAGS "-fsanitize=undefined,address")
set(LSAN_OPTIONS "verbosity=1:log_threads=1")

# This needs at least gcc9 for this
SET(CMAKE_CXX_COMPILER /usr/bin/g++-9)
set(GCC_DIALECT_FLAGS "-std=c++2a -fconcepts")

# Change this for release
set(CMAKE_BUILD_TYPE "release")

if(CMAKE_BUILD_TYPE MATCHES "[Rr][Ee][Ll][Ee][Aa][Ss][Ee]")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS}")
elseif(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]" OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS} ${GCC_SANITIZE_FLAGS} ${GCC_DEBUG_FLAGS}")
elseif(CMAKE_BUILD_TYPE MATCHES "[Pp][Rr][Oo][Ff][Ii][Ll][Ee]")
    set(CMAKE_CXX_FLAGS "-pg ${CMAKE_CXX_FLAGS} -O1 -DNDEBUG ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS}")
endif()

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE HDRS *.h)

set(UTILS_HEADERS "../utils")

message(STATUS "Globbed files ${SRCS} ${HDRS}")

find_package(Curses REQUIRED)

add_executable(ASCII-UML-Generator ${SRCS} ${HDRS})
target_link_libraries(ASCII-UML-Generator ${CURSES_LIBRARIES} pthread)
target_include_directories(ASCII-UML-Generator PUBLIC include ${CURSES_INCLUDE_DIR} ${UTILS_HEADERS})
