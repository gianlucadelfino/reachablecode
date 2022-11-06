#pragma once

#include <atomic>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

#include "control_block.h"
#include "file_descriptor.h"
#include "mapped_memory.h"
#include "queue_base.h"

/**
 * @brief The queue reader accesses the shared memory queue. It can be used
 * concurently with the writer, but only one writer and one reader can be
 * writing/reading on the same queue at the same time.
 */
class queue_reader : public detail::queue_base<queue_reader>
{
public:
  struct const_view
  {
    explicit operator bool() const { return _ptr; }

    size_t size() const { return _len; }

    const char* data() const { return _ptr; }

  private:
    friend queue_reader;
    const_view(const char* ptr, size_t size) : _ptr(ptr), _len(size) {}
    const char* _ptr;
    const size_t _len;
  };

  /**
   * @brief get_buffer checks if there are "bytes_to_read" avaiable to read in
   * the queue and returns a const view to the message. The returned value can
   * be "falsy" if there are not "bytes_to_read" to read.
   *
   * @param bytes_to_read: the number of bytes to read.
   */
  const_view get_buffer(size_t bytes_to_read);

  /**
   * @brief After calling get_buffer and having used the view to read the
   * message from it, the user *has* to call pop to advance the
   * reader pointer.
   *
   * @param bytes_to_pop: It could be a smaller amount of the number passed
   * to get_buffer, meaning the user could pop in "chunks" if they wanted to,
   * but it cannot be more than the total number of bytes passed to all the
   * calls to get_buffer since the last pop.
   */
  void pop(size_t bytes_to_pop);

private:
  queue_reader(mapped_memory&& control_block_region,
               mapped_memory&& first_mapping,
               mapped_memory&& second_mapping,
               size_t size);

  friend queue_base<queue_reader>;

  const char* get_reader_ptr(size_t bytes_to_read);

private:
  control_block& _header;
  mapped_memory _control_block_region;
  mapped_memory _first_mapping;
  mapped_memory _second_mapping;
  const size_t _size;
  size_t _uncommitted_reads{};
};
