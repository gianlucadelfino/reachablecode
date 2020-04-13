#pragma once

/**
 * VideoWindow is a quick utility to create a window in openCV
 */
class VideoWindow
{
public:
    VideoWindow(int deviceID_, const std::string& window_name_)
        : _window_name(window_name_),
          _cap(deviceID_),
          _brightness{
              static_cast<int>(_cap.get(CV_CAP_PROP_BRIGHTNESS) * 100.0)},
          _contrast{static_cast<int>(_cap.get(CV_CAP_PROP_CONTRAST) * 100.0)},
          _saturation{
              static_cast<int>(_cap.get(CV_CAP_PROP_SATURATION) * 100.0)},
          _exposure([this] {
              // where 0.25 means "manual exposure, manual iris"
              _cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25);
              return static_cast<int>(_cap.get(CV_CAP_PROP_EXPOSURE) * 100.0);
          }()),
          _fps{static_cast<int>(_cap.get(cv::CAP_PROP_FPS))}
    {
        // Check if camera opened successfully
        if (!_cap.isOpened())
        {
            throw std::runtime_error("Error opening video stream");
        }

        cv::namedWindow(_window_name.c_str());

        // TODO: set via button HD/FULLHD
        _cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
        _cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

        cv::createTrackbar(
            "Brightness",
            _window_name.c_str(),
            std::addressof(_brightness),
            100,
            +[](int value, void* cap_ptr) -> void {
                static_cast<cv::VideoCapture*>(cap_ptr)->set(
                    CV_CAP_PROP_BRIGHTNESS, static_cast<double>(value) / 100.0);
            },
            std::addressof(_cap));

        cv::createTrackbar(
            "Contrast",
            _window_name.c_str(),
            &_contrast,
            100,
            +[](int value, void* cap_ptr) -> void {
                static_cast<cv::VideoCapture*>(cap_ptr)->set(
                    CV_CAP_PROP_CONTRAST, static_cast<double>(value) / 100.0);
            },
            &_cap);

        cv::createTrackbar(
            "Saturation",
            _window_name.c_str(),
            &_saturation,
            100,
            +[](int value, void* cap_ptr) -> void {
                static_cast<cv::VideoCapture*>(cap_ptr)->set(
                    CV_CAP_PROP_SATURATION, static_cast<double>(value) / 100.0);
            },
            &_cap);

        cv::createTrackbar(
            "Exposure (Max value depends on FPS)",
            _window_name.c_str(),
            &_exposure,
            100,
            +[](int value, void* cap_ptr) -> void {
                static_cast<cv::VideoCapture*>(cap_ptr)->set(
                    CV_CAP_PROP_EXPOSURE, static_cast<double>(value) / 100.0);
            },
            &_cap);

        cv::createTrackbar(
            "Frame Per Seconds)",
            _window_name.c_str(),
            &_fps,
            30,
            +[](int value, void* cap_ptr) -> void {
                static_cast<cv::VideoCapture*>(cap_ptr)->set(
                    CV_CAP_PROP_FPS, static_cast<double>(value));
            },
            &_cap);
    }

    cv::Mat getFrame()
    {
        cv::Mat frame;
        _cap >> frame;
        return frame;
    }

    std::string getWindowName() const { return _window_name; }

    ~VideoWindow() { cv::destroyWindow(_window_name); }

private:
    const std::string _window_name;
    cv::VideoCapture _cap;
    int _brightness;
    int _contrast;
    int _saturation;
    int _exposure;
    int _fps;
};

