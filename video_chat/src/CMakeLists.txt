file(GLOB LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     ${CMAKE_BINARY_DIR}/bindings/*cpp)

add_library(video_chat_utils STATIC ${LIB_SOURCES})

target_link_libraries(
  video_chat_utils
  PRIVATE
  tensorflowlite::tensorflowlite
  imgui::imgui
  opencv::opencv
  Threads::Threads
  ffmpeg::avdevice
  ffmpeg::avfilter
  ffmpeg::avformat
  ffmpeg::avcodec
  ffmpeg::swscale
  ffmpeg::swresample
  ffmpeg::postproc
  ffmpeg::avutil
  GLEW::GLEW
  glfw::glfw
)
