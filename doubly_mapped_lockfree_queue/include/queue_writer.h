#pragma once

#include "queue_base.h"
#include <atomic>
#include <cstdint>

/**
 * @brief The queue writer accesses the shared memory queue. It can be used
 * concurently with the reader, but only one writer and one reader can be
 * writing/reading on the same queue at the same time.
 */
class queue_writer : public detail::queue_base<queue_writer>
{
public:
  struct mutable_view
  {
    explicit operator bool() const { return _ptr; }

    size_t size() const { return _len; }

    char* data() { return _ptr; }

  private:
    mutable_view(char* ptr, size_t size) : _ptr(ptr), _len(size) {}
    friend queue_writer;

    char* _ptr;
    const size_t _len;
  };

  mutable_view get_buffer(size_t bytes_to_write);

  /**
   * @brief After calling get_buffer and having used the buffer to copy the
   * message into it, the user *has* to call push to advance the
   * writer pointer.
   *
   * @param bytes_to_push: It could be a smaller amount of the number passed
   * to get_buffer, meaning the user could push in "chunks" if they wanted to,
   * but it cannot be more than the total number of bytes passed to all the
   * calls to get_buffer since the last push.
   */
  void push(size_t bytes_to_push);

private:
  char* get_writer_ptr(size_t bytes_to_write);

  queue_writer(mapped_memory&& control_block_region,
               mapped_memory&& first_mapping,
               mapped_memory&& second_mapping,
               size_t size);

  friend queue_base<queue_writer>;

private:
  control_block& _header;
  mapped_memory _control_block_region;
  mapped_memory _first_mapping;
  mapped_memory _second_mapping;
  const size_t _size;
  size_t _uncommitted_writes{};
};