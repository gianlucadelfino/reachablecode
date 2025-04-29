cmake_minimum_required(VERSION 3.24.1)

project(cUDP)

# This will display the full g++ command in the output.
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined,address")

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

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

FetchContent_Declare(asio
  GIT_REPOSITORY git@github.com:chriskohlhoff/asio.git
  GIT_TAG asio-1-34-2
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)

FetchContent_MakeAvailable(asio)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
find_package(Threads)
target_link_libraries(asio INTERFACE Threads::Threads)

# set the 3rd parties include
set(3RD_PARTIES_INCLUDE "../3rdParties")

# ftrapv checks for overflow and crashes the program if it happens.
set(GCC_DEBUG_FLAGS "-O0 -g -ftrapv -fsanitize=undefined,address")

# set the common include
set(COMMON_INCLUDE "${CMAKE_SOURCE_DIR}/../common/include")

set(SRCSSender src/sender.cpp)
set(SRCSReceiver src/receiver.cpp)

add_executable(cUDPsender ${SRCSSender})

add_executable(cUDPreceiver ${SRCSReceiver})

target_link_libraries(cUDPsender asio opencv_core opencv_highgui opencv_dnn ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(cUDPreceiver asio opencv_core opencv_highgui opencv_dnn ${CMAKE_THREAD_LIBS_INIT})

include_directories(SYSTEM ${3RD_PARTIES_INCLUDE} ${COMMON_INCLUDE})

include_directories(PRIVATE
  "include"
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
