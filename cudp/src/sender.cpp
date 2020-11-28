#include "Logger.h"
#include "opencv2/opencv.hpp"
#include <chrono>
#include <iostream>
#include <map>
#include <thread>

#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/host_name.hpp>
#include <asio/ip/udp.hpp>

#include "InputBuffer.h"
#include "LockFreeSpsc.h"
#include "OpenCVUtils.h"
#include "TimeLogger.h"
#include "VideoWindow.h"

void sender(const std::string& recv_address_)
{
  try
  {
    ::asio::io_context ioContext;

    ::asio::ip::udp::resolver resolver(ioContext);

    ::asio::ip::udp::resolver::query query(::asio::ip::host_name(), "");
    //    const auto addr = resolver.resolve(query)->endpoint().address();
    //    std::cout<<"USing "<< addr.to_string()<<std::endl;

    ::asio::ip::udp::resolver::iterator it = resolver.resolve(query);

    while (it != ::asio::ip::udp::resolver::iterator())
    {
      auto addr = (it++)->endpoint().address();

      std::cout << addr.to_string() << std::endl;
    }

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

    // Aim for 24 fps
    const auto ms_per_update =
        std::chrono::milliseconds(static_cast<int>(1000 / 24.f));

    while (true)
    {
      const auto timepoint_before_compression =
          std::chrono::system_clock::now();
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

      Logger::Debug("Frame Size", buffer.size());
      const int16_t parts_num =
          (buffer.size() + InputBuffer::writable_size() - 1) /
          InputBuffer::writable_size();
      Logger::Debug("Frame id", frame_id, "split in parts", parts_num);
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
                                 InputBuffer::writable_size()));

        std::error_code err;
        sender_socket.send_to(input_buffer.buffer(), recv_endpoint, 0, err);
        if (err)
        {
          std::cerr << "ERR sending " << err.message() << std::endl;
        }
      }

      // Display the resulting frame
      opencv_utils::displayMat(cv::imdecode(buffer, cv::IMREAD_UNCHANGED),
                               win.getWindowName());

      const auto timepoint_after_sending = std::chrono::system_clock::now();
      auto frame_duration =
          (timepoint_after_sending - timepoint_before_compression);

      if (frame_duration < ms_per_update)
      {
        auto sleep_duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                ms_per_update - frame_duration);

        Logger::Debug("Sleep for", sleep_duration.count());
        std::this_thread::sleep_for(sleep_duration);
      }

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
  Logger::SetLevel(Logger::DEBUG);
  std::string recv_address;
  std::cout << ::asio::ip::host_name() << std::endl;
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
