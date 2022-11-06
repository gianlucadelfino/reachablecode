#include <atomic>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

#include "queue_reader.h"

const char* queue_reader::get_reader_ptr(size_t bytes_to_read)
{
  if (bytes_to_read + _uncommitted_reads > _size)
  {
    return nullptr;
  }

  const uint64_t cur_reader_offset = _header.next_read_offset.load(std::memory_order_relaxed);
  const uint64_t cur_next_write_offset = _header.next_write_offset.load(std::memory_order_acquire);
  assert(cur_reader_offset < _size);
  assert(cur_next_write_offset < _size);

  // Distinguish 2 cases
  size_t readable_bytes{};
  if (cur_reader_offset <= cur_next_write_offset)
  {
    // |--r-w--|-------|
    readable_bytes = cur_next_write_offset - cur_reader_offset;
  }
  else // if (cur_reader_offset > cur_next_write_offset)
  {
    // |--w-r--|-------|
    readable_bytes = cur_next_write_offset + (_size - cur_reader_offset);
  }

  if (bytes_to_read + _uncommitted_reads > readable_bytes)
  {
    return nullptr;
  }

  const char* reader_ptr = _first_mapping.get_address() + cur_reader_offset + _uncommitted_reads;

  _uncommitted_reads += bytes_to_read;
  return reader_ptr;
}

queue_reader::const_view queue_reader::get_buffer(size_t bytes_to_read)
{
  const char* reader_ptr = get_reader_ptr(bytes_to_read);
  if (reader_ptr)
  {
    return {reader_ptr, bytes_to_read};
  }
  else
  {
    return {nullptr, 0};
  }
}

void queue_reader::pop(size_t bytes_to_commit)
{
  assert(bytes_to_commit < _size);
  assert(_uncommitted_reads >= bytes_to_commit);
  const uint64_t cur_reader_offset = _header.next_read_offset.load(std::memory_order_relaxed);

  uint64_t new_reader_offset = cur_reader_offset + bytes_to_commit;
  if (new_reader_offset >= _size)
  {
    // This is crossing over the boundary, then we need to scale it back of "size"
    new_reader_offset -= _size;
  }
  assert(new_reader_offset < _size);

  _header.next_read_offset.store(new_reader_offset, std::memory_order_release);
  _uncommitted_reads -= bytes_to_commit;
}

queue_reader::queue_reader(mapped_memory&& control_block_region,
                           mapped_memory&& first_mapping,
                           mapped_memory&& second_mapping,
                           size_t size)
    : _header(control_block_region.get_header<control_block>()),
      _control_block_region(std::move(control_block_region)),
      _first_mapping(std::move(first_mapping)),
      _second_mapping(std::move(second_mapping)),
      _size(size)
{
}
