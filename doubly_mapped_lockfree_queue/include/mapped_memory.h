#pragma once

#include <cassert>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

#include "file_descriptor.h"

/**
 * @brief We use this class to abstract the mmap functions in a nice RAII way.
 * The destructor will call munmap.
 */
class mapped_memory
{
public:
  mapped_memory(const std::string& filename,
                size_t expected_size,
                void* start_addr = nullptr,
                size_t offset = 0);

  mapped_memory(size_t expected_size, void* start_addr = nullptr, size_t offset = 0);

  mapped_memory(const mapped_memory&) = delete;
  mapped_memory& operator=(const mapped_memory&) = delete;

  mapped_memory(mapped_memory&& other) noexcept { *this = std::move(other); }
  mapped_memory& operator=(mapped_memory&& other) noexcept;

  /**
   * @brief After this returns the destructor will not munmap this region.
   */
  void release() noexcept;

  // Getters
  char* get_address() { return static_cast<char*>(_mapped_region); }
  const char* get_address() const { return static_cast<const char*>(_mapped_region); }
  size_t get_length() const { return _length; }

  /**
   * @brief This calls msync on the mmapped area.
   * Mind that this function can be expensive and it's not suggested to
   * call it in the hot path
   */
  void msync(bool sync)
  {
    [[maybe_unused]] const int err = ::msync(_mapped_region, _length, sync ? MS_SYNC : MS_ASYNC);
    assert(err == 0);
  }

  template <typename ControlBlock>
  ControlBlock& get_header()
  {
    // We want to make sure we are at least not reinterpret casting something
    // that is not non-trivial, however we can only assume "is_standard_layout",
    // as std::atomics are not trivial anymore:
    // https://stackoverflow.com/questions/29759441/is-a-class-with-deleted-copy-constructor-trivially-copyable/29759556#29759556
    static_assert(std::is_standard_layout<ControlBlock>::value,
                  "ControlBlock type must be standard layout");

    assert(_mapped_region);
    assert(_length >= sizeof(ControlBlock));

    // NB: This is technically UB, as no one ever called the ControlBlock's
    // constructor. This will hopefully be fixed with std::bless/std::start_lifetime_as
    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0593r4.html#166x-implicit-object-creation-ptrbless
    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2590r0.pdf
    return *reinterpret_cast<ControlBlock*>(_mapped_region);
  }

  ~mapped_memory();

private:
  static void* map_memory(size_t expected_size,
                          void* start_addr = nullptr,
                          int fd = 0,
                          size_t offset = 0);

  void* _mapped_region{};
  size_t _length{};
};
