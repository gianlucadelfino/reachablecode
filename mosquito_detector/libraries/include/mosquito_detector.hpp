#pragma once

#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>

#include "opencv2/core.hpp"

#include "VideoWindow.h"
#include "OpenCVUtils.h"

class mosquito_detector
{
  public:
  explicit mosquito_detector(int deviceID_) : _window(deviceID_, "feed") {}

  void start()
  {
    // _thread = std::move(std::thread([this]()
    {
      std::vector<cv::Mat> images_to_average;

      while (_running)
      {
        cv::Mat frame = _window.getFrame();
        if (!frame.empty())
        {
            // opencv_utils::displayMat(frame, _window.getWindowName());
          if (images_to_average.size() != _num_of_images_in_average)
          {
            images_to_average.push_back(frame);
            opencv_utils::displayMat(frame, _window.getWindowName());
            const float progess = static_cast<float>(images_to_average.size())/_num_of_images_in_average;
            std::cout << "Building average background.. Progress: " << progess*100 << "%\r" << std::flush;
          }
          else if (images_to_average.size() == _num_of_images_in_average && _start_average.empty())
          {
            _start_average = get_average(images_to_average);
            std::cout << "Building average background.. Done!" << std::endl;
          }
        }
        opencv_utils::displayMat(frame, _window.getWindowName());
        if (!_start_average.empty())
        {
          opencv_utils::displayMat(_start_average, "averaged background");
        }

        // std::this_thread::sleep_for(std::chrono::milliseconds(30));

        if (cv::waitKey(10) >= 0)
        {
          _running = false;
        }
      }
    }
    // ));
  }

  ~mosquito_detector()
  {
    if(_thread.joinable())
    {
      _thread.join();
    }

  }

  private:

  cv::Mat get_average(const std::vector<cv::Mat>& images)
  {
      if (images.empty()) return cv::Mat();

      // Create a 0 initialized image to use as accumulator
      cv::Mat m(images[0].rows, images[0].cols, CV_64FC3);
      m.setTo(cv::Scalar(0,0,0,0));

      // Use a temp image to hold the conversion of each input image to CV_64FC3
      // This will be allocated just the first time, since all your images have
      // the same size.
      cv::Mat temp;
      for (int i = 0; i < images.size(); ++i)
      {
          // Convert the input images to CV_64FC3 ...
          images[i].convertTo(temp, CV_64FC3);

          // ... so you can accumulate
          m += temp;
      }

      // Convert back to CV_8UC3 type, applying the division to get the actual mean
      m.convertTo(m, CV_8U, 1. / images.size());
      return m;
  }

  VideoWindow _window;
  const size_t _num_of_images_in_average{100};
  cv::Mat _start_average;
  std::mutex _m;
  std::thread _thread;
  std::atomic_bool _running{true};
};
