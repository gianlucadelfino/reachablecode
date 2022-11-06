#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

#include "control_block.h"
#include "mapped_memory.h"

namespace detail
{

/**
 * @brief A utility base class that builds the double mapping for both the
 * queue_reader and writer. This is part of the implementation and should not
 * be instantiated by the user.
 */
template <typename Derived>
class queue_base
{
public:
  static Derived queue_factory(const std::string& queue_filename,
                               const std::string& control_block_filename)
  {
    const size_t size = std::filesystem::file_size(queue_filename);
    assert(size);

    // Check that the size is a multiple of the page_size, this will allow
    // us to map in nicely
    [[maybe_unused]] const size_t page_size = getpagesize();
    assert(!(size % page_size));

    // Let's reserve the space before we mmap the buffer twice.
    mapped_memory double_mapping(2 * size);

    // Now we do the mapping of the same file twice in the contiguous region
    // we reserved. This will invalidate the previous mapping btw.
    mapped_memory first_mapping(queue_filename, size, double_mapping.get_address());
    assert(double_mapping.get_address() == first_mapping.get_address());
    assert(first_mapping.get_length() == size);
    mapped_memory second_mapping(
        queue_filename, size, first_mapping.get_address() + first_mapping.get_length());

    double_mapping.release();

    mapped_memory control_block_region(control_block_filename, sizeof(control_block));
    return Derived(
        std::move(control_block_region), std::move(first_mapping), std::move(second_mapping), size);
  }

protected:
  ~queue_base() = default; // Prevent direct instatiation
};
} // namespace detail
