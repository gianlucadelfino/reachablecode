#pragma once

#include <atomic>
#include <new>

#ifdef __cpp_lib_hardware_interference_size
using std::hardware_destructive_interference_size;
#else
// 64 bytes on x86-64 │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned │ ...
constexpr std::size_t hardware_destructive_interference_size = 64;
#endif

/**
 * @brief This is the control block of our shared memory queue.
 * It's using alignas to make sure the two indices are not on the same cache
 * line. This is so that the reader and the writer can each work on its own
 * cache line
 */
struct control_block
{
  alignas(hardware_destructive_interference_size) std::atomic<uint64_t> version;
  std::atomic<uint64_t> next_read_offset;
  alignas(hardware_destructive_interference_size) std::atomic<uint64_t> next_write_offset;
};
