#include "opencv2/opencv.hpp"
#include <chrono>
#include <iostream>
#include <map>
#include <thread>

#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include "LockFreeSpsc.h"
#include "Logger.h"
#include "OpenCVUtils.h"
#include "TimeLogger.h"
#include "VideoWindow.h"

const int MB = 1024 * 1024;

std::vector<uchar> global_test_buffer;

struct InputBuffer
{
  constexpr static int64_t MTU = 1500;

  InputBuffer() = default;
  InputBuffer(const InputBuffer&) = default;

  InputBuffer(InputBuffer&&) = default;

  InputBuffer& operator=(InputBuffer&&) = default;

  struct Header
  {
    int32_t frame_id{};
    int32_t part_begin{};
    int16_t part_id{};
    int16_t total_parts{};
    int32_t part_size{};

    friend std::ostream& operator<<(std::ostream& o, const Header& h)
    {
      o << "Frame id: " << h.frame_id << ", part num: " << h.part_id
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

  std::pair<Header, ::asio::const_buffer> parse() const
  {
    Header h = get_header();
    return std::make_pair(h, get_frame_part(h.part_size));
  }

  static size_t size() { return MTU; }

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
  ::asio::const_buffer get_frame_part(size_t part_size_) const
  {
    return ::asio::const_buffer(_recv_buff.data() + sizeof(Header), part_size_);
  }
  std::array<char, MTU> _recv_buff{0};
};

struct FrameStitcher
{
  explicit FrameStitcher(int parts_num_)
      : _parts_num(parts_num_), _image_buffer(5 * MB)
  {
  }

  FrameStitcher(FrameStitcher&&) = default;

  void add(const InputBuffer::Header& h_, ::asio::const_buffer part_)
  {
    assert(_parts_num > 0);
    assert(h_.part_begin + part_.size() <= _image_buffer.size());

    memcpy(_image_buffer.data() + h_.part_begin, part_.data(), part_.size());
    _parts_num--;
    std::cout << "parts num " << _parts_num << std::endl;
  }

  bool is_complete() const { return _parts_num == 0; }

  cv::Mat decoded() const
  {
    assert(is_complete());

//    std::cout <<"CHECKING BUFFER " << _image_buffer.size() << " vs " << global_test_buffer.size() << std::endl;
//    for (size_t i =0; i < 50; ++i)
    {
//        if (_image_buffer[i] != global_test_buffer[i])
//        {
//            std::cout << "Found diff " << i << std::endl;
//            break;;
//        }
    }

    return cv::imdecode(_image_buffer, cv::IMREAD_UNCHANGED);
  }

  private:
  int _parts_num;
  std::vector<uchar> _image_buffer;
};

struct FramesManager
{
  void add(const InputBuffer::Header& h_, ::asio::const_buffer part_)
  {
    if (h_.frame_id < _last_frame_id - 2)
    {
      std::cerr << "too old " << h_.frame_id << std::endl;
      // Too old, throw it away
      return;
    }
    else
    {
      //      std::cerr << "got frame id " << h_.frame_id << "_last_frame id "
      //      << _last_frame_id << std::endl;
      // Throw away the old one and start with the new one
      _last_frame_id = std::max(_last_frame_id, h_.frame_id);
      _frames.erase(_last_frame_id - 2);
    }

    auto frameIter = _frames.find(h_.frame_id);
    if (frameIter == _frames.cend())
    {
      TimeLogger t("New frame found", std::cout);
      frameIter = _frames
                      .insert(std::make_pair(h_.frame_id,
                                             FrameStitcher(h_.total_parts)))
                      .first;
    }

    auto& frameStitcher = frameIter->second;

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

    LockFreeSpsc<InputBuffer> disruptor(10000);

    struct Handler
    {
      Handler(::asio::ip::udp::socket& socket_,
              InputBuffer& input_buffer_,
              LockFreeSpsc<InputBuffer>& disruptor_)
          : _socket(socket_),
            _input_buffer(input_buffer_),
            _disruptor(disruptor_)
      {
      }

      void operator()(asio::error_code err, std::size_t)
      {
        if (err)
        {
          Logger::Error("Recv Err ", err.message());
        }
        else
        {
          static bool frame_dropped{};
          InputBuffer::Header header = _input_buffer.get_header();

          if (header.part_id == 0)
          {
            // Try again with new frame
            frame_dropped = false;
          }

          if (!frame_dropped && !_disruptor.try_push(std::move(_input_buffer)))
          {
            // Couldn't insert this part, let's skip all the rest of the parts
            // until the next frame
            frame_dropped = true;
          }

          _socket.async_receive(_input_buffer.data(), *this);
        }
        //              cv::waitKey(1);
      }

  private:
      ::asio::ip::udp::socket& _socket;
      InputBuffer& _input_buffer;
      LockFreeSpsc<InputBuffer>& _disruptor;
    };

    Handler handler(recv_socket, input_buffer, disruptor);
    recv_socket.async_receive(input_buffer.data(), handler);

    std::thread display_frame_thread([&disruptor] {
      try
      {
        while (true)
        {
          static FramesManager frame_manager;
          static cv::Mat frame;
          //          TimeLogger t("Recv part", std::cout);

          InputBuffer part_buf;
          while (disruptor.try_pop(part_buf))
          {
            auto [header, part] = part_buf.parse();
            Logger::Debug("Received", header);

            frame_manager.add(header, part);

            if (frame_manager.is_frame_ready())
            {
              Logger::Debug("Updating Frame");
              frame = frame_manager.get_last_frame();
              std::cout << "UPDATED FRAME" << std::endl;
            }
          }

          std::cout << "is frame empty? " << frame.empty() << std::endl;
          if (!frame.empty())
          {
            std::cout << "displaying frame" << std::endl;
            opencv_utils::displayMat(frame, "recv");
          }

          // Press  ESC on keyboard to  exit
//          const char c = static_cast<char>(cv::waitKey(1));
          std::this_thread::sleep_for(std::chrono::milliseconds(16));
//          if (c == 27)
          {
//            break;
          }
        }
      }
      catch (const std::exception& e_)
      {
        Logger::Error("Receiver Error: ", e_.what());
      }
    });

    ioContext.run();
    if (display_frame_thread.joinable())
    {
      display_frame_thread.join();
    }
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
        50); // that's percent, so 100 == no compression

    std::vector<uchar> buffer(20 * MB);

    while (true)
    {
      cv::Mat frame = // cv::imread("/home/gianluca/dev/cppfiddler/exshelf/build/shelfTest2_small.jpg", cv::IMREAD_COLOR);

       win.getFrame();
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
        global_test_buffer = buffer; // REMOVE
      }

      std::cout << "buf size " << buffer.size() << std::endl;
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

        //        Logger::Debug("Part begin", h.part_begin, "Part Size",
        //        part_size);

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

//      std::this_thread::sleep_for(std::chrono::milliseconds(10));

      // Press  ESC on keyboard to  exit
      const char c = static_cast<char>(cv::waitKey(0));
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

  std::thread recv_thread(receiver);

  sender();

  if (recv_thread.joinable())
    recv_thread.join();
  return 0;
}
