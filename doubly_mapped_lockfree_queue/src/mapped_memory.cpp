#include "mapped_memory.h"

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

#include "file_descriptor.h"

mapped_memory::mapped_memory(const std::string& filename,
                             size_t expected_size,
                             void* start_addr,
                             size_t offset)
    : _length(expected_size)
{
  file_descriptor file(filename, expected_size, O_CREAT | O_RDWR);

  _mapped_region = map_memory(expected_size, start_addr, file.fd(), offset);
}

mapped_memory::mapped_memory(size_t expected_size, void* start_addr, size_t offset)
    : _mapped_region(map_memory(expected_size, start_addr, 0, offset)), _length(expected_size)
{
}

mapped_memory& mapped_memory::operator=(mapped_memory&& other) noexcept
{
  std::swap(_mapped_region, other._mapped_region);
  std::swap(_length, other._length);
  return *this;
}

void mapped_memory::release() noexcept
{
  _mapped_region = nullptr;
  _length = 0;
}

mapped_memory::~mapped_memory()
{
  if (_mapped_region)
  {
    msync(true /*synchronous*/);
    ::munmap(_mapped_region, _length);
  }
}

void* mapped_memory::map_memory(size_t expected_size, void* start_addr, int fd, size_t offset)
{
  [[maybe_unused]] const size_t page_size = getpagesize();
  // assert(!(expected_size % page_size));

  // Start address must be aligned for our logic to work, though mmap would work regardless.
  // We need to reliably be able to build two contiguous mappings
  assert(!(reinterpret_cast<std::uintptr_t>(start_addr) % page_size));

  const int protect_flags{PROT_READ | PROT_WRITE};

  int flags{};
  if (start_addr)
  {
    // In case we provide a starting address we impose map_fixed to
    // impose that we want that specific address.
    flags |= MAP_FIXED;
  }

  if (fd)
  {
    flags |= MAP_SHARED_VALIDATE;
  }
  else
  {
    flags |= MAP_SHARED | MAP_ANONYMOUS;
  }

  void* const mapped_region = ::mmap(start_addr, expected_size, protect_flags, flags, fd, offset);

  if (mapped_region == MAP_FAILED)
  {
    throw std::runtime_error("cannot mmap memory. Error: " + std::to_string(errno));
  }

  if (start_addr && start_addr != mapped_region)
  {
    throw std::runtime_error("Asked for a specific address, but mapped on another");
  }

  return mapped_region;
}
