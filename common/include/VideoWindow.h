#pragma once

#include <exception>
#include <string>

#include "opencv2/opencv.hpp"

/**
 * @brief Window is a RAII class to call destroyWindow for us
 */
class Window
{
  public:
  explicit Window(const std::string& window_name_) : _window_name(window_name_)
  {
    if (window_name_.empty())
    {
      throw std::runtime_error("We cannot instantiate a window without a name");
    }
    cv::namedWindow(_window_name.c_str(), cv::WINDOW_AUTOSIZE);
  }

  std::string getWindowName() const { return _window_name; }

  ~Window()
  {
    try
    {
      cv::destroyWindow(_window_name);
    }
    catch (const std::exception& e_)
    {
      std::cerr << "Exception while destroying window " << _window_name
                << ". Error is " << e_.what() << std::endl;
    }
  }

  private:
  const std::string _window_name;
};

/**
 * VideoWindow is a quick utility to create a video window in openCV
 */
class VideoWindow
{
  public:
  VideoWindow(int deviceID_, const std::string& window_name_)
      : _window(window_name_),
        _cap(deviceID_),
        _brightness{
            static_cast<int>(_cap.get(cv::CAP_PROP_BRIGHTNESS) * 100.0)},
        _contrast{static_cast<int>(_cap.get(cv::CAP_PROP_CONTRAST) * 100.0)},
        _saturation{
            static_cast<int>(_cap.get(cv::CAP_PROP_SATURATION) * 100.0)},
        _exposure([this] {
          // where 0.25 means "manual exposure, manual iris"
          _cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25);
          return static_cast<int>(_cap.get(cv::CAP_PROP_EXPOSURE) * 100.0);
        }()),
        _fps{static_cast<int>(_cap.get(cv::CAP_PROP_FPS))}
  {
    // Check if camera opened successfully
    if (!_cap.isOpened())
    {
      throw std::runtime_error("Error opening video stream");
    }

    // TODO: set via button HD/FULLHD
    _cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    _cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
  }

  void enableControlBars()
  {
    cv::createTrackbar(
        "Brightness",
        getWindowName().c_str(),
        std::addressof(_brightness),
        100,
        +[](int value, void* cap_ptr) -> void {
          static_cast<cv::VideoCapture*>(cap_ptr)->set(
              cv::CAP_PROP_BRIGHTNESS, static_cast<double>(value) / 100.0);
        },
        std::addressof(_cap));

    cv::createTrackbar(
        "Contrast",
        getWindowName().c_str(),
        &_contrast,
        100,
        +[](int value, void* cap_ptr) -> void {
          static_cast<cv::VideoCapture*>(cap_ptr)->set(
              cv::CAP_PROP_CONTRAST, static_cast<double>(value) / 100.0);
        },
        &_cap);

    cv::createTrackbar(
        "Saturation",
        getWindowName().c_str(),
        &_saturation,
        100,
        +[](int value, void* cap_ptr) -> void {
          static_cast<cv::VideoCapture*>(cap_ptr)->set(
              cv::CAP_PROP_SATURATION, static_cast<double>(value) / 100.0);
        },
        &_cap);

    cv::createTrackbar(
        "Exposure (Max value depends on FPS)",
        getWindowName().c_str(),
        &_exposure,
        100,
        +[](int value, void* cap_ptr) -> void {
          static_cast<cv::VideoCapture*>(cap_ptr)->set(
              cv::CAP_PROP_EXPOSURE, static_cast<double>(value) / 100.0);
        },
        &_cap);

    cv::createTrackbar(
        _fps_bar_name.c_str(),
        getWindowName().c_str(),
        &_fps,
        30,
        +[](int value, void* cap_ptr) -> void {
          static_cast<cv::VideoCapture*>(cap_ptr)->set(
              cv::CAP_PROP_FPS, static_cast<double>(value));
        },
        &_cap);
  }

  cv::Mat getFrame()
  {
    cv::Mat frame;
    _cap >> frame;
    return frame;
  }

  void setFPS(int fps_)
  {
    _cap.set(cv::CAP_PROP_FPS, static_cast<double>(fps_));
    cv::setTrackbarPos(_fps_bar_name.c_str(),
                       _window.getWindowName().c_str(),
                       static_cast<int>(fps_));
  }

  std::string getWindowName() const { return _window.getWindowName(); }

  private:
  const Window _window;
  cv::VideoCapture _cap;
  int _brightness;
  int _contrast;
  int _saturation;
  int _exposure;
  int _fps;

  const std::string _fps_bar_name{"Frames Per Second"};
};
