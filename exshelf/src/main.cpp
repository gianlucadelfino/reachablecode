#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <iostream>
#include <math.h>
#include <mutex>
#include <sstream>
#include <thread>

#include "opencv2/opencv.hpp"

// Requires opencv4
#include <opencv2/dnn.hpp>
#include <tesseract/baseapi.h>

#include "Library.h"
#include "OCRText.h"
#include "OpenCVUtils.h"
#include "VideoWindow.h"
#include "Logger.h"

int main()
{
    // Hardcoded conf
    const float NMSConf = 0.4f;
    const int rescaleWidth = 2 * 320;  // Must be multiple of 32
    const int rescaleHeight = 2 * 320; // Must be multiple of 32

    // Tesseract setup
    tesseract::TessBaseAPI ocr;

    // Change is to the appropriate language
    ocr.Init(nullptr, "ita", tesseract::OEM_LSTM_ONLY);

    ocr.SetPageSegMode(tesseract::PSM_SINGLE_LINE);

    Library lib;

    // Optional webcam input
    // VideoWindow v(0, "webcam");
    // v.setFPS(1);

    bool keep_looping{true};

    // Text detection model: https://github.com/argman/EAST
    // https://www.dropbox.com/s/r2ingd0l3zt8hxs/frozen_east_text_detection.tar.gz?dl=1
    cv::dnn::Net detector = cv::dnn::readNet("frozen_east_text_detection.pb");

    while (keep_looping)
    {
        // Optional webcam input
        // cv::Mat frame = v.getFrame();
        cv::Mat frame = cv::imread(
            "../res/shelfTest2_small.jpg");

        // If the frame is empty, continue
        if (frame.empty())
        {
            Logger::Error("Emtpy frame");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        const double scale =
            static_cast<double>(1000) / static_cast<double>(frame.size().width);
        cv::resize(frame, frame, cv::Size(), scale, scale, cv::INTER_LANCZOS4);

        opencv_utils::displayMat(frame, "ExShelf");

        // Adjust this depending on oriantation of input, or rotate it around
        // 4 times to try all orientations
        cv::rotate(frame, frame, cv::ROTATE_90_CLOCKWISE);

        cv::Mat blob =
            cv::dnn::blobFromImage(frame,
                                   1.0,
                                   cv::Size(rescaleWidth, rescaleHeight),
                                   cv::Scalar(123.68, 116.78, 103.94),
                                   true,
                                   false);

        {
            // Debug, display hte blob
            // cv::Mat green(
            //    rescaleHeight, rescaleWidth, CV_32F, blob.ptr<float>(0,
            //    1));
            // cv::Mat red(
            //    rescaleHeight, rescaleWidth, CV_32F, blob.ptr<float>(0,
            //    2));
            // cv::Mat blue(
            //    rescaleHeight, rescaleWidth, CV_32F, blob.ptr<float>(0,
            //    0));

            // std::vector<cv::Mat> blobChannels{blue, green, red};
            // cv::Mat blobRGB;
            // cv::merge(blobChannels, blobRGB);
            // opencv_utils::displayMat(blobRGB, "blob");
        }

        detector.setInput(blob);
        std::vector<cv::Mat> outs;
        const std::vector<cv::String> outNames(
            {"feature_fusion/Conv_7/Sigmoid", "feature_fusion/concat_3"});

        detector.forward(outs, outNames);

        cv::Mat scores = outs[0];
        cv::Mat geometry = outs[1];

        const float confThreshold = 0.1f;

        auto [boxes, confidences] =
            opencv_utils::decodeBoundingBoxes(scores, geometry, confThreshold);

        std::vector<int> nonSuppressedIndices;
        cv::dnn::NMSBoxes(
            boxes, confidences, confThreshold, NMSConf, nonSuppressedIndices);

        cv::Point2f ratio(static_cast<float>(frame.cols) / rescaleWidth,
                          static_cast<float>(frame.rows) / rescaleHeight);

        std::vector<cv::Rect> nonSuppressedBoxes;
        for (int index : nonSuppressedIndices)
        {
            const cv::RotatedRect& box = boxes[static_cast<size_t>(index)];

            // Get the vertices
            std::array<cv::Point2f, 4> boxVertices{};
            box.points(boxVertices.data());
            for (cv::Point2f& vertex : boxVertices)
            {
                vertex.x *= ratio.x;
                vertex.y *= ratio.y;
            }

            nonSuppressedBoxes.emplace_back(boxVertices[1], boxVertices[3]);
        }

        // DEBUG: Display word rectangles
        {
            // DEBUG: display the box
            cv::Mat copy = frame.clone();
            const cv::Scalar color = cv::Scalar(100, 255, 100);

            for (const auto& rect : nonSuppressedBoxes)
            {
                cv::rectangle(copy, rect, color, 3);
            }
            opencv_utils::displayMat(copy, "words");
        }

        // Joine the nearby words into titles
        std::vector<cv::Rect> joinedRects =
            opencv_utils::joinAlignedRects(nonSuppressedBoxes);

        // DEBUG
        cv::Mat copy = frame.clone();
        for (const auto& rect : joinedRects)
        {
            static const cv::Scalar color = cv::Scalar(100, 255, 100);
            cv::rectangle(copy, rect, color, 3);
        }
        opencv_utils::displayMat(copy, "joinedTitles");

        const auto scanFrameForTitles = [&ocr](const cv::Mat& frame_) {
            ocr.SetImage(frame_.data,
                         frame_.cols,
                         frame_.rows,
                         frame_.channels(),
                         static_cast<int>(frame_.step));
            ocr.SetSourceResolution(300);

            // Debug
            Logger::Debug("full detected text: ", ocr.GetUTF8Text());

            return ocrTextUtils::getBooks(std::unique_ptr<char[]>(ocr.GetUNLVText()),
                                          std::unique_ptr<int[]>(ocr.AllWordConfidences()));
        };

        cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);

        for (auto& bounding_box : joinedRects)
        {
            cv::Mat crop;

            // Add some padding
            const float padding_percent = 0.2f;
            bounding_box.width +=
                static_cast<float>(bounding_box.width) * padding_percent;
            bounding_box.height +=
                static_cast<float>(bounding_box.height) * padding_percent;
            bounding_box.x -=
                static_cast<float>(bounding_box.width) * padding_percent / 2;
            bounding_box.y -=
                static_cast<float>(bounding_box.height) * padding_percent / 2;

            // Fix outbound issues
            bounding_box.x = std::max(bounding_box.x, 0);
            bounding_box.y = std::max(bounding_box.y, 0);
            if (bounding_box.x + bounding_box.width >= frame.size().width)
            {
                bounding_box.width = frame.size().width - bounding_box.x;
            }
            if (bounding_box.y + bounding_box.height >= frame.size().height)
            {
                bounding_box.height = frame.size().height - bounding_box.y;
            }

            frame(bounding_box).copyTo(crop);

            crop = opencv_utils::straighten(crop);

            for (int j = 0; j < 2; ++j)
            {
                cv::bitwise_not(crop, crop);

                const std::vector<std::pair<std::string, float>> books =
                    scanFrameForTitles(crop);

                for (auto& title : books)
                {
                    lib.insert(BookTitle(title.first, title.second));
                }
            }
        }

        auto titles = lib.getTitles();
        Logger::Info("Num titles", titles.size());
        for (auto& title : titles)
        {
            Logger::Info(title);
        }

        const char c = static_cast<char>(cv::waitKey(10));
        if (c == 27)
        {
            keep_looping = false;
        }
    }

    return 0;
}
