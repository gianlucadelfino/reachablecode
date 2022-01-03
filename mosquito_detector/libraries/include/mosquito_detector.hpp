#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

#include "opencv2/core.hpp"

#include "OpenCVUtils.h"
#include "VideoWindow.h"

class mosquito_detector
{
  public:
  explicit mosquito_detector(int deviceID_) : _window(deviceID_, "feed") {}

  void start()
  {
    std::vector<cv::Mat> images_to_average;

    while (_running)
    {
      cv::Mat frame = _window.getFrame();
      if (!frame.empty())
      {
        if (_warmup_frames-- > 0)
        {
          opencv_utils::displayMat(frame, _window.getWindowName());
          continue;
        }

        // opencv_utils::displayMat(frame, _window.getWindowName());
        if (images_to_average.size() != _num_of_images_in_average)
        {
          images_to_average.push_back(frame);
          opencv_utils::displayMat(frame, _window.getWindowName());
          const float progess = static_cast<float>(images_to_average.size()) /
                                _num_of_images_in_average;
          std::cout << "Building average background.. Progress: "
                    << progess * 100 << "%\r" << std::flush;
        }
        else if (images_to_average.size() == _num_of_images_in_average)
        {
          if (_start_average.empty())
          {
            _start_average = opencv_utils::get_average(images_to_average);
          }
          else
          {
            _current_average = opencv_utils::get_average(images_to_average);
          }
          images_to_average.clear();
          std::cout << "Building average background.. Progress: 100%. Done\n"
                    << std::endl;
        }
      }
      opencv_utils::displayMat(frame, _window.getWindowName());
      if (!_start_average.empty())
      {
        opencv_utils::displayMat(_start_average, "start averaged background");
      }

      if (!_current_average.empty())
      {
        opencv_utils::displayMat(_current_average,
                                 "current averaged background");
      }

      if (!_start_average.empty() && !_current_average.empty())
      {
        detect_mosquito(_start_average, _current_average);
      }

      // std::this_thread::sleep_for(std::chrono::milliseconds(30));

      if (cv::waitKey(10) >= 0)
      {
        _running = false;
      }
    }
  }

  private:
  void detect_mosquito(const cv::Mat& average_init_,
                       const cv::Mat& cur_average_)
  {
    cv::Mat average_init_greyscale;
    cv::cvtColor(average_init_, average_init_greyscale, cv::COLOR_BGR2GRAY);

    cv::Mat cur_average_greyscale;
    cv::cvtColor(cur_average_, cur_average_greyscale, cv::COLOR_BGR2GRAY);

    cv::Mat diff_image;
    cv::absdiff(average_init_greyscale, cur_average_greyscale, diff_image);

    cv::threshold(diff_image, diff_image, 20, 0, cv::THRESH_TOZERO);
    opencv_utils::displayMat(diff_image, "diff image");

    const std::vector<cv::Rect> bb = opencv_utils::findBoundingBoxes(diff_image);
  }

  VideoWindow _window;
  const size_t _num_of_images_in_average{100};
  int _warmup_frames{100};
  cv::Mat _start_average;
  cv::Mat _current_average;
  bool _running{true};
};
