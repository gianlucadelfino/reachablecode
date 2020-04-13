#include "opencv2/opencv.hpp"
#include <iostream>
#include <chrono>
#include <thread>

#include "VideoWindow.h"

int main()
{
    VideoWindow win(0, "cUDP");

//    std::vector<int> compression_params;
//    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
//    // TODO: make this dynamic?
//    compression_params.push_back(
//        60); // that's percent, so 100 == no compression

//    const int MB = 1024 * 1024;
//    std::vector<uchar> buffer(20 * MB);

    while (true)
    {
        cv::Mat frame = win.getFrame();

        // If the frame is empty, break immediately
        if (frame.empty())
        {
            std::cerr << "Emtpy frame";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        // cv::imencode(".jpg", frame, buffer, compression_params);
        // std::cout << "size is " << buffer.size() << std::endl;
        // return 0;

        // Display the resulting frame
        cv::imshow(win.getWindowName(), frame);

        // Press  ESC on keyboard to  exit
        const char c = static_cast<char>(cv::waitKey(1));
        if (c == 27)
        {
            break;
        }
    }

    return 0;
}
