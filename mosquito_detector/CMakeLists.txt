cmake_minimum_required(VERSION 3.15)
project(mosquito_detector CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Added manually
set(CONAN_DISABLE_CHECK_COMPILER ON)
set(CONAN_SYSTEM_INCLUDES ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

enable_testing()

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

include_directories(${CMAKE_SOURCE_DIR}/../common/include)

# End added manually

add_executable(mosquito_detector src/mosquito_detector.cpp src/main.cpp)

# Added manually this line too
target_link_libraries(mosquito_detector ${CONAN_LIBS})

install(TARGETS mosquito_detector DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        )
