#pragma once

#include <chrono>
#include <thread>

#include "opencv2/core.hpp"

#include "VideoWindow.h"
#include "OpenCVUtils.h"

class mosquito_detector
{
  public:
  explicit mosquito_detector(int deviceID_) : _window(deviceID_, "feed") {}

  void start()
  {
    while (true)
    {
      cv::Mat frame = _window.getFrame();
      if (!frame.empty())
      {
        opencv_utils::displayMat(frame, _window.getWindowName());
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
      if (cv::waitKey(10) >= 0)
        break;
    }
  }

  private:
  VideoWindow _window;
};
