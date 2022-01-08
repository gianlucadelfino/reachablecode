#pragma once

#include <atomic>
#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

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

      std::this_thread::sleep_for(std::chrono::milliseconds(30));

      if (cv::waitKey(10) >= 0)
      {
        _running = false;
      }
    }
  }

  private:
  static void detect_mosquito(const cv::Mat& average_init_,
                              const cv::Mat& cur_average_)
  {
    cv::Mat average_init_greyscale;
    cv::cvtColor(average_init_, average_init_greyscale, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(average_init_greyscale, average_init_greyscale);

    cv::Mat cur_average_greyscale;
    cv::cvtColor(cur_average_, cur_average_greyscale, cv::COLOR_BGR2GRAY);

    opencv_utils::displayMat(cur_average_greyscale, "before equalizeHist");
    cv::equalizeHist(cur_average_greyscale, cur_average_greyscale);
    opencv_utils::displayMat(cur_average_greyscale, "after equalizeHist");

    std::cout <<"Computing diff" << std::endl;
    cv::Mat diff_image;
    cv::absdiff(average_init_greyscale, cur_average_greyscale, diff_image);
    std::cout <<"Computing threshold" << std::endl;

    cv::threshold(diff_image, diff_image, 20, 0, cv::THRESH_TOZERO);
    opencv_utils::displayMat(diff_image, "diff image");

    std::cout <<"Computing get_bounding_boxes" << std::endl;
    const std::vector<cv::Rect> bb =
        get_bounding_boxes(diff_image, 30);
  }

  static std::vector<cv::Rect> get_bounding_boxes(const cv::Mat& mat_, int max_side_)
  {
    cv::Mat frame = mat_.clone();
    if (frame.channels() > 1)
    {
      cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    }

    cv::blur(frame, frame, cv::Size(2, 2));

    cv::imshow("blur", frame);

    const int threshold = 50;
    cv::Canny(frame, frame, threshold, threshold * 2);

    cv::imshow("canny", frame);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(frame, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point>> contours_polys(contours.size());
    std::vector<cv::Rect> bounding_boxes;

    for (size_t i = 0; i < contours.size(); ++i)
    {
      cv::approxPolyDP(contours[i], contours_polys[i], 3, true);

      cv::Rect bbox = cv::boundingRect(contours_polys[i]);
      // Filter the big ones
      const auto& frame_size = frame.size();
      if (bbox.width < max_side_ and bbox.height < max_side_ )
      {
        bounding_boxes.push_back(bbox);
      }
    }

    // Display bounding boxes
    const int max_display_width = 600;
    frame = mat_.clone();

    for (auto& bounding_box : bounding_boxes)
    {
      static const cv::Scalar color = cv::Scalar(100, 255, 100);
      cv::rectangle(frame, bounding_box.tl(), bounding_box.br(), color, 3);
    }

    if (frame.size().width > max_display_width)
    {
      const double scale = static_cast<double>(max_display_width) /
                           static_cast<double>(frame.size().width);
      cv::resize(frame, frame, cv::Size(), scale, scale, cv::INTER_LANCZOS4);
    }
    static Window win("bounding boxes");
    cv::imshow(win.getWindowName(), frame);

    return bounding_boxes;
  }

  VideoWindow _window;
  const size_t _num_of_images_in_average{100};
  int _warmup_frames{100};
  cv::Mat _start_average;
  cv::Mat _current_average;
  bool _running{true};
};
