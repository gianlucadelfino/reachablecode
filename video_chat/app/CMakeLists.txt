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

if(BUILD_WASM)
  set_target_properties(video_chat PROPERTIES SUFFIX ".html" LINK_FLAGS
                                                                  "--emrun")
  install(
    TARGETS video_chat
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

  install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/video_chat.js
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/video_chat.wasm
          DESTINATION bin)
endif()