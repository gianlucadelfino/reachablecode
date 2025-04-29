cmake_minimum_required(VERSION 3.24.1)

project(ASCII-UML-Generator)

# This will display the full g++ command in the output.
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined,address")

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

file(GLOB_RECURSE HDRS *.h)

set(UTILS_HEADERS "../common/include")

include_directories(${UTILS_HEADERS})

message(STATUS "Globbed files ${SRCS} ${HDRS}")

find_package(Curses REQUIRED)

add_executable(ASCII-UML-Generator src/main.cpp ${HDRS})
target_link_libraries(ASCII-UML-Generator ${CURSES_LIBRARIES} pthread)
target_include_directories(ASCII-UML-Generator PUBLIC include ${CURSES_INCLUDE_DIR} ${UTILS_HEADERS})
