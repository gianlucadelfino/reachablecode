cmake_minimum_required(VERSION 3.24.1)
project(almost_always_lockfree CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

# pthread
find_package(Threads)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

set(FMT_TEST OFF CACHE INTERNAL "disabling fmt tests")
FetchContent_Declare(
        fmt
        GIT_REPOSITORY  https://github.com/fmtlib/fmt.git
        GIT_TAG         9.1.0
        GIT_PROGRESS    TRUE
        USES_TERMINAL_DOWNLOAD TRUE
)
FetchContent_MakeAvailable(fmt)

add_compile_options("$<$<CONFIG:RELEASE>:-Wall;-Wextra;-pedantic;-Werror>")
# ftrapv checks for overflow and crashes the program if it happens.
add_compile_options(
  "$<$<CONFIG:DEBUG>:-O0;-g;-Wall;-Wextra;-pedantic;-Werror;-ftrapv;-fsanitize=undefined,address>"
)

add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=undefined,address>")

include_directories(../common/include)

enable_testing()
add_subdirectory(tests)
