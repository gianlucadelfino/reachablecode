#include "opencv2/opencv.hpp"
#include <chrono>
#include <iostream>
#include <map>
#include <thread>

#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "Logger.h"
#include "OpenCVUtils.h"
#include "VideoWindow.h"

const int MB = 1024 * 1024;

struct InputBuffer
{
  constexpr static int64_t MTU = 1500;

  struct Header
  {
    int32_t frame_id{};
    int32_t part_begin{};
    int16_t part_num{};
    int16_t total_parts{};

    friend std::ostream& operator<<(std::ostream& o, const Header& h)
    {
      o << "Got frame id: " << h.frame_id << ", part num: " << h.part_num
        << ", part begin: " << h.part_begin;
      return o;
    }
  };

  Header get_header() const
  {
    Header h{};
    memcpy(&h, _recv_buff.data(), sizeof(h));
    return h;
  }

  ::asio::const_buffer get_frame_part(size_t recv_bytes) const
  {
    assert(sizeof(Header) < recv_bytes);
    return ::asio::const_buffer(_recv_buff.data() + sizeof(Header),
                                recv_bytes - sizeof(Header));
  }

  void set_header(const Header& h_)
  {
    memcpy(_recv_buff.data(), &h_, sizeof(h_));
  }

  void set_frame_part(::asio::const_buffer buf_)
  {
    assert(buf_.size() <= writable_size());
    memcpy(_recv_buff.data() + sizeof(Header), buf_.data(), buf_.size());
  }

  static size_t writable_size() { return MTU - sizeof(Header); }

  ::asio::mutable_buffer data()
  {
    return ::asio::mutable_buffer(_recv_buff.data(), _recv_buff.size());
  }

  ::asio::const_buffer buffer() const
  {
    return ::asio::const_buffer(_recv_buff.data(), _recv_buff.size());
  }

  private:
  std::array<char, MTU> _recv_buff{0};
};

struct FrameStitcher
{
  explicit FrameStitcher(int parts_num_)
      : _parts_num(parts_num_), _image_buffer(20 * MB)
  {
  }

  FrameStitcher(FrameStitcher&&) = default;

  void add(const InputBuffer::Header& h_, ::asio::const_buffer part_)
  {
    assert(_parts_num > 0);
    assert(h_.part_begin + part_.size() <= _image_buffer.size());
    memcpy(_image_buffer.data() + h_.part_begin, part_.data(), part_.size());
    _parts_num--;

//    std::cout << "parts num "<< _parts_num << std::endl;
  }

  bool is_complete() const { return _parts_num == 0; }

  cv::Mat decoded() const
  {
    return cv::imdecode(_image_buffer,cv::IMREAD_UNCHANGED);
  }

  private:
  int _parts_num;
  std::vector<char> _image_buffer;
};

struct FramesManager
{
  void add(const InputBuffer::Header& h_, ::asio::const_buffer part_)
  {
    if (h_.frame_id < _last_frame_id - 2)
    {
//      std::cerr <<"too old " << h_.frame_id << std::endl;
      // Too old, throw it away
      return;
    }
    else
    {
//      std::cerr << "got frame id " << h_.frame_id << "_last_frame id " << _last_frame_id << std::endl;
      // Throw away the old one and start with the new one
      _last_frame_id = std::max(_last_frame_id, h_.frame_id);
      _frames.erase(_last_frame_id - 2);
    }

    auto frameIter = _frames.insert(
        std::make_pair(h_.frame_id, FrameStitcher(h_.total_parts)));
    auto& frameStitcher = frameIter.first->second;

    frameStitcher.add(h_, part_);

    if (frameStitcher.is_complete())
    {
      if (h_.frame_id >= _last_frame_id)
      {
        _last_complete_frame = h_.frame_id;
      }
      else
      {
        // It's stale, throw it away
        _frames.erase(h_.frame_id);
      }
    }
  }

  bool is_frame_ready() const { return _last_complete_frame != -1; }

  cv::Mat get_last_frame()
  {
    assert(_last_complete_frame > -1);
    FrameStitcher frame_stitcher = std::move(_frames.at(_last_complete_frame));
    _last_complete_frame = -1;
    return frame_stitcher.decoded();
  }

  private:
  int _last_complete_frame{-1};
  int _last_frame_id{};
  std::map<int, FrameStitcher> _frames;
};

void receiver()
{
  try
  {
    ::asio::io_context ioContext;

    ::asio::ip::udp::resolver resolver(ioContext);

    ::asio::ip::udp::socket recv_socket(ioContext);

    const std::string recv_address("0.0.0.0");
    const int recv_port = 39009;

    InputBuffer input_buffer;

    ::asio::ip::udp::endpoint recv_endpoint(::asio::ip::udp::v4(), recv_port);

    recv_socket.open(::asio::ip::udp::v4());

    recv_socket.bind(recv_endpoint);

    struct Handler
    {
      Handler(::asio::ip::udp::socket& socket_, InputBuffer& input_buffer_)
          : _socket(socket_), _input_buffer(input_buffer_)
      {
      }

      void operator()(asio::error_code err, std::size_t size_)
      {
        if (err)
        {
          Logger::Error("Recv Err ", err.message());
        }
        else
        {
          static FramesManager frame_manager;

          InputBuffer::Header header = _input_buffer.get_header();
//          Logger::Debug("Received", header);

          frame_manager.add(header, _input_buffer.get_frame_part(size_));

          static cv::Mat recv_frame;
          if (frame_manager.is_frame_ready())
          {
            Logger::Debug("Updating Frame");
            recv_frame = frame_manager.get_last_frame();
          }

          _socket.async_receive(_input_buffer.data(),
                                Handler(_socket, _input_buffer));
          if (!recv_frame.empty())
          {
            opencv_utils::displayMat(recv_frame, "Recv frame");
          }
        }
        //              cv::waitKey(1);
      }

  private:
      ::asio::ip::udp::socket& _socket;
      InputBuffer& _input_buffer;
    };

    recv_socket.async_receive(input_buffer.data(), Handler(recv_socket, input_buffer));

    ioContext.run();
  }
  catch (const std::exception& e_)
  {
    Logger::Error("Receiver Error: ", e_.what());
  }
}

void sender()
{
  try
  {
    ::asio::io_context ioContext;

    ::asio::ip::udp::resolver resolver(ioContext);

    ::asio::ip::udp::socket sender_socket(ioContext);

    sender_socket.open(::asio::ip::udp::v4());

    const std::string recv_address("0.0.0.0");
    const int recv_port = 39009;
    ::asio::ip::udp::endpoint recv_endpoint(
        ::asio::ip::address::from_string(recv_address), recv_port);

    VideoWindow win(1, "cUDP");
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);

    // TODO: make this dynamic?
    compression_params.push_back(
        60); // that's percent, so 100 == no compression

    std::vector<uchar> buffer(20 * MB);

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
      cv::Mat frame = win.getFrame();
      static int frame_id = 0;


      // If the frame is empty, break immediately
      if (frame.empty())
      {
        std::cerr << "Emtpy frame";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        continue;
      }

      cv::imencode(".jpg", frame, buffer, compression_params);

      //      Logger::Debug("buffer size", buffer.size());

      frame_id++;
      // Split the frame in MTU size parts
      const int16_t parts_num =
          (buffer.size() + InputBuffer::writable_size() - 1) /
          InputBuffer::writable_size();
      for (int16_t part_num = 0; part_num < parts_num; ++part_num)
      {
        //        Logger::Debug("Sending Part", part_num, "of parts",
        //        parts_num);
        InputBuffer::Header h;
        h.part_num = part_num;
        h.total_parts = parts_num;
        // Size is either MTU or the remainder for the last part
        h.frame_id = frame_id;
        h.part_begin = part_num * InputBuffer::writable_size();

        const size_t part_size =
            part_num + 1 == parts_num ?
                buffer.size() % InputBuffer::writable_size() :
                InputBuffer::writable_size();

        //        Logger::Debug("Part begin", h.part_begin, "Part Size",
        //        part_size);

        InputBuffer input_buffer;
        input_buffer.set_header(h);

        assert(h.part_begin + part_size <= buffer.size());
        input_buffer.set_frame_part(
            ::asio::const_buffer(buffer.data() + h.part_begin, part_size));

        std::error_code err;
        sender_socket.send_to(input_buffer.buffer(), recv_endpoint, 0, err);
      }

      // Display the resulting frame
      opencv_utils::displayMat(frame, win.getWindowName());

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

int main(/*int argc, char* argv[]*/)
{
  Logger::SetLevel(Logger::DEBUG);
  //    if (argc < 2)
  //    {
  //        std::cerr << "Please add the remote address " << std::endl;
  //        return -1;
  //    }
  //    const std::string recv_address(argv[1]);

  std::thread sender_thread(receiver);

  sender();

  if (sender_thread.joinable())
    sender_thread.join();
  return 0;
}
