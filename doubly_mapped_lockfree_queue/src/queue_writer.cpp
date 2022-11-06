#include "queue_writer.h"

char* queue_writer::get_writer_ptr(size_t bytes_to_write)
{
  // The writer can only write size-1 at most, this is due to how the queue
  // is designed.
  assert(_size >= 1);
  if (bytes_to_write + _uncommitted_writes > _size - 1)
  {
    return nullptr;
  }

  const uint64_t cur_write_offset = _header.next_write_offset.load(std::memory_order_relaxed);
  const uint64_t cur_next_read_offset = _header.next_read_offset.load(std::memory_order_acquire);
  assert(cur_next_read_offset < _size);
  assert(cur_write_offset < _size);

  // Distinguish 2 cases
  size_t writable_bytes{};
  if (cur_next_read_offset <= cur_write_offset)
  {
    // |--r-w--|-------|
    writable_bytes = (_size - 1 - cur_write_offset) + cur_next_read_offset;
  }
  else // if (cur_next_read_offset > cur_write_offset)
  {
    // |--w-r--|-------|
    writable_bytes = cur_next_read_offset - cur_write_offset - 1;
  }

  if (bytes_to_write + _uncommitted_writes > writable_bytes)
  {
    return nullptr;
  }

  char* writer_ptr = _first_mapping.get_address() + cur_write_offset + _uncommitted_writes;

  _uncommitted_writes += bytes_to_write;
  return writer_ptr;
}

queue_writer::mutable_view queue_writer::get_buffer(size_t bytes_to_write)
{
  char* const writer_ptr = get_writer_ptr(bytes_to_write);
  if (writer_ptr)
  {
    return {writer_ptr, bytes_to_write};
  }
  return {nullptr, 0};
}

void queue_writer::push(size_t bytes_to_commit)
{
  assert(_uncommitted_writes >= bytes_to_commit);
  const uint64_t cur_write_offset = _header.next_write_offset.load(std::memory_order_relaxed);

  uint64_t new_next_write_offset = cur_write_offset + bytes_to_commit;
  if (new_next_write_offset >= _size)
  {
    // This is crossing over the boundary, then we need to scale it back of "size"
    new_next_write_offset -= _size;
  }
  assert(new_next_write_offset < _size);

  _header.next_write_offset.store(new_next_write_offset, std::memory_order_release);
  _uncommitted_writes -= bytes_to_commit;
}

queue_writer::queue_writer(mapped_memory&& control_block_region,
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
