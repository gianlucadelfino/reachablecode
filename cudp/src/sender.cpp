#include "opencv2/opencv.hpp"
#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include "Logger.h"

#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "LockFreeSpsc.h"
#include "OpenCVUtils.h"
#include "TimeLogger.h"
#include "VideoWindow.h"
#include "InputBuffer.h"

void sender(const std::string& recv_address_)
{
  try
  {
    ::asio::io_context ioContext;

    ::asio::ip::udp::resolver resolver(ioContext);

    ::asio::ip::udp::socket sender_socket(ioContext);

    sender_socket.open(::asio::ip::udp::v4());

    const int recv_port = 39009;
    ::asio::ip::udp::endpoint recv_endpoint(
        ::asio::ip::address::from_string(recv_address_), recv_port);

    VideoWindow win(1, "cUDP");
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);

    // TODO: make this dynamic?
    compression_params.push_back(
        50); // that's percent, so 100 == no compression

    std::vector<uchar> buffer(20 * MB);

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

      static int frame_id = 0;

      {
        TimeLogger t("Encoding frame", std::cout);
        cv::imencode(".jpg", frame, buffer, compression_params);
      }

      frame_id++;

      const int16_t parts_num =
          (buffer.size() + InputBuffer::writable_size() - 1) /
          InputBuffer::writable_size();
      for (int16_t part_id = 0; part_id < parts_num; ++part_id)
      {
        InputBuffer::Header h;
        h.part_id = part_id;
        h.total_parts = parts_num;
        // Size is either MTU or the remainder for the last part
        h.frame_id = frame_id;
        h.part_begin = part_id * InputBuffer::writable_size();

        h.part_size = part_id + 1 == parts_num ?
                          buffer.size() % InputBuffer::writable_size() :
                          InputBuffer::writable_size();

        InputBuffer input_buffer;
        input_buffer.set_header(h);
        input_buffer.set_frame_part(
            ::asio::const_buffer(reinterpret_cast<const char*>(buffer.data()) +
                                     part_id * InputBuffer::writable_size(),
                                 h.part_size));

        std::error_code err;
        sender_socket.send_to(input_buffer.buffer(), recv_endpoint, 0, err);
        if (err)
        {
          std::cerr << "ERR sending " << err.message() << std::endl;
        }
      }

      // Display the resulting frame
      opencv_utils::displayMat(frame, win.getWindowName());

      std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 24));

      // Press  ESC on keyboard to  exit
      const char c = static_cast<char>(cv::waitKey(1));
      if (c == 27)
      {
        break;
      }
    }
  }
  catch (const std::exception& e_)
  {
    Logger::Error("Sernder Error: ", e_.what());
  }
}

int main(int argc, char* argv[])
{
  Logger::SetLevel(Logger::INFO);
  std::string recv_address;
  if (argc < 2)
  {
      Logger::Warning("No Address passed, using localhost");
      recv_address = "127.0.0.1";
  }
  else
  {
    recv_address = argv[1];
  }

  sender(recv_address);
  return 0;
}
