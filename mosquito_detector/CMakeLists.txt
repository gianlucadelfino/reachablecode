cmake_minimum_required(VERSION 3.10.0)
project(mosquito_detector CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)

# pthread
find_package(Threads)

if(WIN32)
    # This can be any version of opencv
    set(OpenCV_DIR "C:/OpenCV-4.5.5/")
endif()

# OpenCv
set(OpenCV_STATIC ON)
find_package(OpenCV REQUIRED)

# set the common include
set(COMMON_INCLUDE "${CMAKE_SOURCE_DIR}/../common/include")

set(CMAKE_CXX_STANDARD 20)

if(NOT WIN32)
  # NB Blacklist for libfontconfig
  configure_file(sanitizer_blacklist.txt sanitizer_blacklist.txt COPYONLY)

  # ftrapv checks for overflow and crashes the program if it happens.
  set(GCC_DEBUG_FLAGS "-O0 -g -ftrapv")
  set(GCC_SANITIZE_FLAGS "-fsanitize=undefined,address")
  set(LSAN_OPTIONS "verbosity=1:log_threads=1")
  set(GCC_WARNING_FLAGS "-Wall -Werror -Wextra -pedantic")

  if(CMAKE_BUILD_TYPE MATCHES "[Rr][Ee][Ll][Ee][Aa][Ss][Ee]")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_WARNING_FLAGS} ${GCC_DIALECT_FLAGS} ${GCC_SANITIZE_FLAGS} ${GCC_DEBUG_FLAGS}")
  endif()
else()
    add_compile_options(
        $<$<CONFIG:>:/MT> #---------|
        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MT> #--|
    )
endif()

enable_testing()

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libraries)
include_directories(${LIB_DIR}/include)

# add_subdirectory(tests)
add_subdirectory(apps)

add_subdirectory(libraries)