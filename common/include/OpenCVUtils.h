#pragma once

#include <mutex>

#include "opencv2/opencv.hpp"

#include "Logger.h"
#include "Math.h"
#include "VideoWindow.h"

namespace opencv_utils
{

inline void displayMat(cv::Mat mat, const std::string& windowName_, float scale=1.f)
{
  static std::mutex display_mat;
  std::lock_guard<std::mutex> l(display_mat);

  cv::Mat frame = mat.clone();
  static std::map<std::string, Window> winMap;

  auto winIter = winMap.find(windowName_);
  if (winIter == winMap.end())
  {
    winIter = winMap
                  .emplace(std::piecewise_construct,
                           std::forward_as_tuple(windowName_),
                           std::forward_as_tuple(windowName_))
                  .first;
  }

  auto& win = winIter->second;

  if (scale > 1.f or scale < 1.f)
  {
    cv::resize(frame, frame, cv::Size(), scale, scale, cv::INTER_LANCZOS4);
  }

  const int max_display_width = 1920;
  if (frame.size().width > max_display_width)
  {
    cv::Mat resized;
    const double max_scale = static_cast<double>(max_display_width) /
                             static_cast<double>(frame.size().width);
    cv::resize(frame, resized, cv::Size(), max_scale, max_scale, cv::INTER_LANCZOS4);
    cv::imshow(win.getWindowName().c_str(), resized);
  }
  else
  {
    cv::imshow(win.getWindowName().c_str(), frame);
  }
}

// Adapted from
// https://docs.opencv.org/master/db/da4/samples_2dnn_2text_detection_8cpp-example.html
inline
std::pair<std::vector<cv::RotatedRect>, std::vector<float>> decodeBoundingBoxes(
    const cv::Mat& scores, const cv::Mat& geometry, float scoreThresh)
{
  std::pair<std::vector<cv::RotatedRect>, std::vector<float>> detectionsConfs;
  std::vector<cv::RotatedRect>& detections = detectionsConfs.first;
  std::vector<float>& confidences = detectionsConfs.second;

  CV_Assert(scores.dims == 4);
  CV_Assert(geometry.dims == 4);
  CV_Assert(scores.size[0] == 1);
  CV_Assert(geometry.size[0] == 1);
  CV_Assert(scores.size[1] == 1);
  CV_Assert(geometry.size[1] == 5);
  CV_Assert(scores.size[2] == geometry.size[2]);
  CV_Assert(scores.size[3] == geometry.size[3]);

  const int height = scores.size[2];
  const int width = scores.size[3];
  for (int y = 0; y < height; ++y)
  {
    const float* scoresData = scores.ptr<float>(0, 0, y);
    const float* x0_data = geometry.ptr<float>(0, 0, y);
    const float* x1_data = geometry.ptr<float>(0, 1, y);
    const float* x2_data = geometry.ptr<float>(0, 2, y);
    const float* x3_data = geometry.ptr<float>(0, 3, y);
    const float* anglesData = geometry.ptr<float>(0, 4, y);
    for (int x = 0; x < width; ++x)
    {
      const float score = scoresData[x];
      if (score < scoreThresh)
        continue;

      // Decode a prediction.
      // Multiple by 4 because feature maps are 4 time less than input
      // image.
      const float offsetX = x * 4.0f, offsetY = y * 4.0f;
      const float angle = anglesData[x];

      const float cosA = std::cos(angle);
      const float sinA = std::sin(angle);
      const float h = x0_data[x] + x2_data[x];
      const float w = x1_data[x] + x3_data[x];

      const cv::Point2f offset(offsetX + cosA * x1_data[x] + sinA * x2_data[x],
                               offsetY - sinA * x1_data[x] + cosA * x2_data[x]);
      const cv::Point2f p1 = cv::Point2f(-sinA * h, -cosA * h) + offset;
      const cv::Point2f p3 = cv::Point2f(-cosA * w, sinA * w) + offset;
      const cv::RotatedRect rect(0.5f * (p1 + p3),
                                 cv::Size2f(w, h),
                                 -angle * 180.0f / static_cast<float>(CV_PI));

      detections.push_back(rect);
      confidences.push_back(score);
    }
  }

  return detectionsConfs;
}

// Find the bounding boxes see:
// https://github.com/opencv/opencv/blob/3.4/samples/cpp/tutorial_code/ShapeDescriptors/generalContours_demo1.cpp
inline
std::vector<cv::Rect> findBoundingBoxes(const cv::Mat& mat)
{
  cv::Mat frame = mat.clone();
  if (frame.channels() > 1)
  {
    cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  }

  cv::blur(frame, frame, cv::Size(2, 2));

  const int threshold = 50;
  cv::Canny(frame, frame, threshold, threshold * 2);

  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(frame, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

  std::vector<std::vector<cv::Point>> contours_polys(contours.size());
  std::vector<cv::Rect> bounding_boxes;

  for (size_t i = 0; i < contours.size(); ++i)
  {
    cv::approxPolyDP(contours[i], contours_polys[i], 3, true);

    cv::Rect bbox = cv::boundingRect(contours_polys[i]);
    // Filter the smaller and huge ones
    const auto& size = frame.size();
    if (bbox.width > size.width / 20 and bbox.height > size.height / 20)
    {
      bounding_boxes.push_back(bbox);
    }
  }

  // Display bounding boxes
  const int max_display_width = 1200;
  frame = mat.clone();

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
  static Window win("BoundinBoxes");
  cv::imshow(win.getWindowName(), frame);

  return bounding_boxes;
}

// http://felix.abecassis.me/2011/09/opencv-detect-skew-angle/
// https://docs.opencv.org/2.4/doc/tutorials/imgproc/imgtrans/hough_lines/hough_lines.html
inline
cv::Mat straighten(cv::Mat frame)
{
  cv::Mat orig = frame.clone();

  if (frame.channels() > 1)
  {
    cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  }

  cv::Canny(frame, frame, 50, 150, 3);

  std::vector<cv::Vec4i> lines;
  cv::HoughLinesP(frame, lines, 1, CV_PI / 180, 50, 50, 10);

  if (lines.empty())
  {
    return orig;
  }

  std::vector<double> angles;
  for (const auto& line : lines)
  {
    const double angle =
        atan2(line[3] - line[1], line[2] - line[0]) / CV_PI * 180.0;

    cv::line(frame,
             cv::Point(line[0], line[1]),
             cv::Point(line[2], line[3]),
             cv::Scalar(255, 0, 0),
             3);

    angles.push_back(angle);
  }

  // Debug
  static Window win("HoughLines");
  const int max_display_width = 1200;
  if (frame.size().width > max_display_width)
  {
    cv::Mat resized;
    const double scale = static_cast<double>(max_display_width) /
                         static_cast<double>(frame.size().width);
    cv::resize(frame, resized, cv::Size(), scale, scale, cv::INTER_LANCZOS4);
    cv::imshow(win.getWindowName().c_str(), resized);
  }
  else
  {
    cv::imshow(win.getWindowName().c_str(), frame);
  }

  std::vector<std::vector<double>> binned_angles =
      math::bin(angles.cbegin(), angles.cend(), 15.0);

  const std::vector<double>& most_common_bin =
      *std::max_element(binned_angles.cbegin(),
                        binned_angles.cend(),
                        [](const auto& vecA, const auto& vecB) {
                          return vecA.size() < vecB.size();
                        });

  // Get the average angle
  const double angle =
      math::average(most_common_bin.cbegin(), most_common_bin.cend());

  // We just want to straighten, not to rotate: even if it's upside-
  // down it doesn't matter as we'll rotate 3 times 90 degrees later.
  // So only want to rotate to the closest 90 degree angle
  const double adjusted_angle = [a = angle]() {
    double angle = std::fmod(a + 180.0, 90.0);
    if (angle > 45.0)
    {
      angle = 90 - angle;
    }

    return -angle;
  }();

  Logger::Debug("Angle ", adjusted_angle);

  cv::Point2f axis(frame.cols / 2.f, frame.rows / 2.f);
  cv::Mat rot = cv::getRotationMatrix2D(axis, adjusted_angle, 1.0);

  cv::warpAffine(orig, orig, rot, orig.size());

  return orig;
};

/**
 * @brief joinAlignedRects gets a list of rectangles some of which are aligned
 *  and close to one another in a line, i.e. they are words of the same title.
 *  We find the aligned word-rectangles and we join them in bigger rectagles,
 *  hopefully including the whole title.
 *
 *  We could do this in many ways, but we will do it assuming the titles are
 *  horizontally oriented, which means we assume that books that are rotated
 *  90 degrees do not get detected together. This function should be called
 *  for each 90 degrees orientation.
 * @param rects_
 * @return a list of fewer and bigger (i.e. joined) rects.
 */
inline
std::vector<cv::Rect> joinAlignedRects(const std::vector<cv::Rect>& rects_)
{
  std::vector<cv::Rect> joinedAlignedRects;

  if (rects_.empty())
  {
    return joinedAlignedRects;
  }

  // We assume book titles are horizontally oriented and group books with
  // similar y coordinate

  auto sorted_rects = rects_;
  std::sort(sorted_rects.begin(),
            sorted_rects.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.y < rhs.y; });

  float last_y = sorted_rects.front().y;
  float last_height = sorted_rects.front().height;
  std::vector<cv::Point> cluster;

  for (const auto& rect : sorted_rects)
  {
    if (rect.y > (last_y + last_height / 2) or
        rect.y < (last_y - last_height / 2))
    {
      joinedAlignedRects.push_back(cv::boundingRect(cluster));
      cluster.clear();
    }

    cluster.push_back(rect.br());
    cluster.push_back(rect.tl());
    last_y = rect.y;
    last_height = rect.height;
  }

  // Check if there is something left in cluster
  if (!cluster.empty())
  {
    joinedAlignedRects.push_back(cv::boundingRect(cluster));
  }

  return joinedAlignedRects;
}

} // namespace opencv_utils
