#pragma once

#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include <array>

const int MB = 1024 * 1024;

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

