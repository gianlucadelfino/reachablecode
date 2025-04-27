cmake_minimum_required(VERSION 3.6)

project(exshelf)

# This will display the full g++ command in the output.
set(CMAKE_VERBOSE_MAKEFILE ON)

# Dipendencies
find_package(PkgConfig REQUIRED)

# pthread
find_package(Threads)

include(FetchContent)

# OpenCv
FetchContent_Declare(
        opencv
        GIT_REPOSITORY https://github.com/opencv/opencv.git
        GIT_TAG        4.11.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(opencv)

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
set(GCC_DEBUG_FLAGS "-O0 -g -ftrapv -fsanitize=undefined,address")

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

include_directories(SYSTEM ${LOCAL_INCLUDE} ${3RD_PARTIES_INCLUDE} ${TESSERACT_INCLUDE_DIR} ${COMMON_INCLUDE})

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

target_include_directories(exshelf PRIVATE
  ${opencv_SOURCE_DIR}/include
  ${OPENCV_CONFIG_FILE_INCLUDE_DIR}
  ${OPENCV_MODULE_opencv_core_LOCATION}/include
  ${OPENCV_MODULE_opencv_highgui_LOCATION}/include
  ${OPENCV_MODULE_opencv_calib3d_LOCATION}/include
  ${OPENCV_MODULE_opencv_features2d_LOCATION}/include
  ${OPENCV_MODULE_opencv_flann_LOCATION}/include
  ${OPENCV_MODULE_opencv_dnn_LOCATION}/include
  ${OPENCV_MODULE_opencv_imgcodecs_LOCATION}/include
  ${OPENCV_MODULE_opencv_videoio_LOCATION}/include
  ${OPENCV_MODULE_opencv_imgproc_LOCATION}/include
  ${OPENCV_MODULE_opencv_ml_LOCATION}/include
  ${OPENCV_MODULE_opencv_objdetect_LOCATION}/include
  ${OPENCV_MODULE_opencv_photo_LOCATION}/include
  ${OPENCV_MODULE_opencv_stitching_LOCATION}/include
  ${OPENCV_MODULE_opencv_video_LOCATION}/include
)

target_link_libraries(exshelf opencv_core opencv_highgui opencv_dnn ${OpenCV_LIBS} ${CLANG_LINK_LIBS} ${TESSERACT_LIBRARIES} ${MATH_LIBS} ${CMAKE_THREAD_LIBS_INIT})
