#pragma once

#include <atomic>
#include <iostream>
#include <new>
#include <type_traits>
#include <vector>

#ifdef __cpp_lib_hardware_interference_size
using std::hardware_destructive_interference_size;
#else
// 64 bytes on x86-64 │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned │
// ...
constexpr std::size_t hardware_destructive_interference_size = 64;
#endif

#define CACHE_LINE_SIZE 128

// A simplified version of folly::ProducerConsumerQueue
template <typename T>
class lockfree_spsc
{
public:
  explicit lockfree_spsc(size_t ring_size_)
      : _next_read_idx(0),
        _next_write_idx(0),
        _ring(ring_size_ + 1) // one is wasted.
  {
    // TODO: Use concepts instead
    static_assert(std::is_move_assignable_v<T> &&
                  std::is_move_constructible_v<T>);
  }

  lockfree_spsc(const lockfree_spsc&) = delete;
  lockfree_spsc(lockfree_spsc&&) = delete;

  bool try_push(T&& t_)
  {
    const int cur_writer_idx = _next_write_idx.load(std::memory_order_relaxed);
    const int cur_reader_idx = _next_read_idx.load(std::memory_order_acquire);

    int new_writer_idx = cur_writer_idx + 1;
    if (new_writer_idx == static_cast<int>(_ring.size()))
    {
      new_writer_idx = 0;
    }

    if (new_writer_idx == cur_reader_idx)
    {
      return false;
    }
    else
    {
      _ring[cur_writer_idx] = std::move(t_);
      _next_write_idx.store(new_writer_idx, std::memory_order_release);

      return true;
    }
  }

  bool try_pop(T& t_)
  {
    const int cur_reader_idx = _next_read_idx.load(std::memory_order_relaxed);
    const int cur_writer_idx = _next_write_idx.load(std::memory_order_acquire);

    if (cur_reader_idx == cur_writer_idx)
    {
      return false;
    }

    int new_reader_idx = cur_reader_idx + 1;
    if (new_reader_idx == static_cast<int>(_ring.size()))
    {
      new_reader_idx = 0;
    }

    // Got something to read
    t_ = std::move(_ring[cur_reader_idx]);
    _next_read_idx.store(new_reader_idx, std::memory_order_release);

    return true;
  }

  void print()
  {
    std::cout << "Ring "
              << ", cur r " << _next_read_idx << ", cur w " << _next_write_idx
              << std::endl;
    for (size_t i = 0; i < _ring.size(); ++i)
    {
      std::cout << _ring[i] << std::endl;
    }
  }

private:
  alignas(hardware_destructive_interference_size) std::atomic_uint64_t
      _next_read_idx{};
  alignas(hardware_destructive_interference_size) std::atomic_uint64_t
      _next_write_idx{};

  std::vector<T> _ring;
};
