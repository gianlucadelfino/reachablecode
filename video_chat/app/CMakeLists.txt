add_executable(video_chat main.cpp)

target_link_libraries(
  video_chat
  PRIVATE
  ${CMAKE_DL_LIBS}
  video_chat_utils
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
