#include "opencv2/opencv.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    const std::string window_name("cUDP");
    cv::namedWindow(window_name.c_str());

    // Create a VideoCapture object and use camera to capture the video
    cv::VideoCapture cap(0);

    // Check if camera opened successfully
    if (!cap.isOpened())
    {
        std::cout << "Error opening video stream" << std::endl;
        return -1;
    }

    int brightness{static_cast<int>(cap.get(CV_CAP_PROP_BRIGHTNESS) * 100.0)};
    int contrast{static_cast<int>(cap.get(CV_CAP_PROP_CONTRAST) * 100.0)};
    int saturation{static_cast<int>(cap.get(CV_CAP_PROP_SATURATION) * 100.0)};

    cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25); // where 0.25 means "manual exposure, manual iris"
    int exposure{static_cast<int>(cap.get(CV_CAP_PROP_EXPOSURE) * 100.0)};

    int fps{static_cast<int>(cap.get(cv::CAP_PROP_FPS))};


    // TODO: set via button HD/FULLHD
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

    cv::createTrackbar(
        "Brightness",
        window_name.c_str(),
        &brightness,
        100,
        +[](int value, void* cap_ptr) -> void {
            static_cast<cv::VideoCapture*>(cap_ptr)->set(
                CV_CAP_PROP_BRIGHTNESS, static_cast<double>(value) / 100.0);
        },
        &cap);

     cv::createTrackbar(
         "Contrast",
         window_name.c_str(),
         &contrast,
         100,
         +[](int value, void* cap_ptr) -> void {
             static_cast<cv::VideoCapture*>(cap_ptr)->set(
                 CV_CAP_PROP_CONTRAST, static_cast<double>(value) / 100.0);
         },
         &cap);

     cv::createTrackbar(
         "Saturation",
         window_name.c_str(),
         &saturation,
         100,
         +[](int value, void* cap_ptr) -> void {
             static_cast<cv::VideoCapture*>(cap_ptr)->set(
                 CV_CAP_PROP_SATURATION, static_cast<double>(value) / 100.0);
         },
         &cap);

     cv::createTrackbar(
         "Exposure (Max value depends on FPS)",
         window_name.c_str(),
         &exposure,
         100,
         +[](int value, void* cap_ptr) -> void {
             static_cast<cv::VideoCapture*>(cap_ptr)->set(
                 CV_CAP_PROP_EXPOSURE, static_cast<double>(value) / 100.0);
         },
         &cap);


     cv::createTrackbar(
         "Frame Per Seconds)",
         window_name.c_str(),
         &fps,
         30,
         +[](int value, void* cap_ptr) -> void {
             static_cast<cv::VideoCapture*>(cap_ptr)->set(
                 CV_CAP_PROP_FPS, static_cast<double>(value));
         },
         &cap);


    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    // TODO: make this dynamic?
    compression_params.push_back(
        60); // that's percent, so 100 == no compression

    const int MB = 1024 * 1024;
    std::vector<uchar> buffer(20 * MB);

    while (true)
    {
        cv::Mat frame;

        // Capture frame-by-frame
        cap >> frame;

        // If the frame is empty, break immediately
        if (frame.empty())
            break;

        // cv::imencode(".jpg", frame, buffer, compression_params);
        // std::cout << "size is " << buffer.size() << std::endl;
        // return 0;

        // Display the resulting frame
        cv::imshow(window_name, frame);

        // Press  ESC on keyboard to  exit
        const char c = static_cast<char>(cv::waitKey(1));
        if (c == 27)
        {
            break;
        }
    }

    // When everything done, release the video capture and write object
    cap.release();

    // Closes all the windows
    cv::destroyAllWindows();
    return 0;
}
