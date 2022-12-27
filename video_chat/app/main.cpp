// extern "C" {
// #include<libavcodec/avcodec.h>

// #include<libavutil/opt.h>
// #include<libavutil/imgutils.h>
// }
// #include <tensorflow/lite/interpreter.h>

#include <exception>
#include <iostream>
#include <memory>

#include "image_utils.h"
#include "imgui_window_manager.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <array>
#include <chrono>
#include <stdexcept>
#include <thread>

#include "VideoWindow.h"

int main()
{
  const int win_width = 1280;
  const int win_height = 720;
  const int webcam_device_id = 0;
  const std::chrono::milliseconds expected_frame_duration(32); // 30fps

  try
  {
    cv::VideoCapture webcam(webcam_device_id);

    imgui_window_manager window_manager("Test win", win_width, win_height);

    window_manager.add_ui_elem(
        [&webcam, buf = std::array<char, 20>{}, texture = utils::gen_texture()]() mutable
        {
          cv::Mat frame;
          webcam >> frame;

          ImGui::Text("This is me");

          if (!frame.empty())
          {
            utils::load_texture(frame, texture);

            ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(texture)),
                         ImVec2(frame.cols, frame.rows));
          }
          else
          {
            ImGui::Text("No image");
          }

          ImGui::InputText("string", buf.data(), buf.size());
          if (ImGui::Button("Call"))
          {
            // do stuff
          }
        });

    while (window_manager.is_running())
    {
      const auto frame_init = std::chrono::system_clock::now();

      window_manager.render();

      const auto frame_end = std::chrono::system_clock::now();
      const std::chrono::milliseconds frame_duration =
          std::chrono::duration_cast<std::chrono::milliseconds>(frame_end - frame_init);
      if (frame_duration < expected_frame_duration)
      {
        std::this_thread::sleep_for(expected_frame_duration - frame_duration);
      }
    }
    return 0;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
