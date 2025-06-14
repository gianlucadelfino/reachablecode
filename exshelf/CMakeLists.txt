cmake_minimum_required(VERSION 3.24.1)

project(exshelf)

# This will display the full g++ command in the output.
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined,address")

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_CXX_STANDARD 17)

# Dependencies
find_package(PkgConfig REQUIRED)

# pthread
find_package(Threads)

include(FetchContent)

# Fetch Leptonica first (Tesseract depends on it)
FetchContent_Declare(
  leptonica
  GIT_REPOSITORY https://github.com/DanBloomberg/leptonica.git
  GIT_TAG        1.85.0  # Use a valid tag or branch
)
FetchContent_MakeAvailable(leptonica)

# OpenCv
FetchContent_Declare(
        opencv
        GIT_REPOSITORY https://github.com/opencv/opencv.git
        GIT_TAG        4.11.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(opencv)

FetchContent_Declare(
  tesseract
  GIT_REPOSITORY https://github.com/tesseract-ocr/tesseract.git
  GIT_TAG        5.5.0
)
set(BUILD_TRAINING_TOOLS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(tesseract)

# Suppress warning errors for tesseract only
if(TARGET libtesseract)
  target_compile_options(libtesseract PRIVATE
    -Wno-unused-parameter
    -Wno-error=unused-parameter
    -Wno-error=maybe-uninitialized
    -Wno-error=stringop-truncation
  )
endif()

# set the 3rd parties include
set(3RD_PARTIES_INCLUDE "${CMAKE_SOURCE_DIR}/../3rdParties")

set(LOCAL_INCLUDE "include")

# set the common include
set(COMMON_INCLUDE "${CMAKE_SOURCE_DIR}/../common/include")

configure_file(res/shelfTest2_small.jpg shelfTest2_small.jpg COPYONLY)

set(SRCS src/main.cpp)

add_executable(exshelf ${SRCS} ${HDRS})

include_directories(SYSTEM ${LOCAL_INCLUDE} ${3RD_PARTIES_INCLUDE} ${COMMON_INCLUDE})

target_include_directories(exshelf PRIVATE
  ${tesseract_BINARY_DIR}/include
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

target_link_libraries(exshelf PRIVATE libtesseract opencv_core opencv_highgui opencv_dnn ${TESSERACT_LIBRARIES} ${MATH_LIBS} ${CMAKE_THREAD_LIBS_INIT})
