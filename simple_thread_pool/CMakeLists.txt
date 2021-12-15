cmake_minimum_required(VERSION 3.10.0)
project(simple_thread_pool)

# Dependencies
enable_testing()
find_package(GTest REQUIRED)

# pthread
find_package (Threads)

set(GCC_DIALECT_FLAGS "-std=c++20")

set(GCC_WARNING_FLAGS "-Wall -Werror -Wextra -pedantic")

# ftrapv checks for overflow and crashes the program if it happens.
set(GCC_DEBUG_FLAGS "-O0 -g -ftrapv")
set(GCC_SANITIZE_FLAGS "-fsanitize=undefined,address")
set(LSAN_OPTIONS "verbosity=1:log_threads=1")

if(CMAKE_BUILD_TYPE MATCHES "[Rr][Ee][Ll][Ee][Aa][Ss][Ee]")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS}")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS} ${GCC_SANITIZE_FLAGS} ${GCC_DEBUG_FLAGS}")
endif()

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libraries)

include_directories(${LIB_DIR}/include)

add_subdirectory(tests)
add_subdirectory(apps)
add_subdirectory(libraries)