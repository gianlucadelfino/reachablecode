#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>

#include <fcntl.h>
#include <unistd.h>

class file_descriptor
{
public:
  explicit file_descriptor(const std::string& filepath,
                           size_t expected_size,
                           int flags = O_CREAT | O_RDWR)
      : _fd(::open(filepath.c_str(), flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH))
  {
    if (_fd == -1)
    {
      throw std::runtime_error("Could not open file: " + filepath +
                               ". Error: " + std::to_string(errno));
    }

    if (::ftruncate(_fd, expected_size) != 0)
    {
      throw std::runtime_error("cannot resize file: " + filepath + " to expected size " +
                               std::to_string(expected_size) + ". Error: " + std::to_string(errno));
    }
  }

  int fd() const { return _fd; }

  ~file_descriptor() { close(_fd); }

private:
  const int _fd{};
};