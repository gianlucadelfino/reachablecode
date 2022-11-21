#include <bits/types/FILE.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <random>

#include "gtest/gtest.h"

#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include "Logger.h"
#include "control_block.h"
#include "file_descriptor.h"
#include "mapped_memory.h"
#include "queue_reader.h"
#include "queue_writer.h"

namespace
{
struct file_cleaner
{
  file_cleaner(const std::string& filename) : _filename(filename)
  {
    std::filesystem::remove(filename);
  }

  ~file_cleaner() { std::filesystem::remove(_filename); }

private:
  const std::string _filename;
};

} // namespace

TEST(mapped_memory, write_and_read)
{
  const std::string control_block_file = "control_block1.bin";
  file_cleaner cleaner(control_block_file);

  {
    mapped_memory control_block_region(control_block_file, sizeof(control_block));

    control_block& header = control_block_region.get_header<control_block>();
    header.next_read_offset = 2;
    control_block_region.msync(true);

    header.next_write_offset = 3;
    control_block_region.msync(true);
  }

  {
    mapped_memory control_block_region(control_block_file, sizeof(control_block));

    control_block& header = control_block_region.get_header<control_block>();
    EXPECT_EQ(2, header.next_read_offset);
    EXPECT_EQ(3, header.next_write_offset);
  }
}

TEST(mapped_memory, write_and_read_different_process)
{
  const std::string control_block_file = "control_block2.bin";
  file_cleaner cleaner(control_block_file);

  const int pid = fork();

  if (pid != 0)
  {
    // Parent
    int child_ret_value{};
    waitpid(pid, &child_ret_value, 0);
    mapped_memory control_block_region(control_block_file, sizeof(control_block));

    control_block& header = control_block_region.get_header<control_block>();
    EXPECT_EQ(2, header.next_read_offset);
    EXPECT_EQ(3, header.next_write_offset);
  }
  else
  {
    mapped_memory control_block_region(control_block_file, sizeof(control_block));

    control_block& header = control_block_region.get_header<control_block>();
    header.next_read_offset = 2;
    control_block_region.msync(true);

    header.next_write_offset = 3;
    control_block_region.msync(true);
    exit(0);
  }
}

TEST(mapped_memory, write_and_read_from_two_contiguous_regions)
{
  const std::string data_file = "data1.bin";
  const size_t size = getpagesize();
  file_cleaner cleaner(data_file);

  mapped_memory reserved_space(2 * size);

  mapped_memory first_mapping(data_file, size, reserved_space.get_address());
  EXPECT_EQ(size, first_mapping.get_length());

  mapped_memory second_mapping(
      data_file, size, reserved_space.get_address() + first_mapping.get_length());

  // Now the area is mapped twice, but with contiguous mappings.
  // I should be able to write and read from either mappings

  char* first_mapping_begin = first_mapping.get_address();
  EXPECT_NE(nullptr, first_mapping_begin);
  *first_mapping_begin = '5';

  char* second_mapping_begin = second_mapping.get_address();

  EXPECT_EQ(second_mapping_begin, first_mapping_begin + size);
  EXPECT_NE(nullptr, second_mapping_begin);

  EXPECT_EQ('5', *second_mapping_begin);

  *(first_mapping_begin + first_mapping.get_length()) = '3';

  // This has to be released manually otherwise it'll be released
  // twice
  reserved_space.release();

  EXPECT_EQ('3', *second_mapping_begin);
}

std::string get_queue_content(size_t size)
{
  assert(!(size % 8));
  std::string data;
  data.reserve(size);
  while (data.size() < size)
  {
    data += "12345678";
  }
  return data;
}

TEST(Queue, queue_read)
{
  const std::string queue_filepath = "queue2.bin";
  file_cleaner cleaner(queue_filepath);

  constexpr size_t queue_size = 4096 * 10;
  file_descriptor queue_file(queue_filepath, queue_size);

  {
    // Manually write something on the file to read it with our reader
    std::fstream file(queue_filepath);
    const std::string data = get_queue_content(queue_size);
    file.write(data.c_str(), data.size());
  }

  const std::string control_block_file = "control_block2.bin";
  file_cleaner cleaner_2(control_block_file);

  queue_reader reader = queue_reader::queue_factory(queue_filepath, control_block_file);

  // Writer is at zero so nothing should come here
  {
    queue_reader::const_view reader_buf = reader.get_buffer(1);
    EXPECT_EQ(false, static_cast<bool>(reader_buf));
  }

  // I should not be able to write the full queue. One slot is lost in the way
  // the queue is designed
  {
    queue_writer writer = queue_writer::queue_factory(queue_filepath, control_block_file);
    ASSERT_FALSE(writer.get_buffer(queue_size));
  }

  {
    queue_writer writer = queue_writer::queue_factory(queue_filepath, control_block_file);
    ASSERT_TRUE(writer.get_buffer(queue_size - 1));
    writer.push(queue_size - 1);
  }

  {
    size_t read_bytes{};
    while (true)
    {
      queue_reader::const_view reader_buf = reader.get_buffer(8);
      if (!reader_buf)
      {
        EXPECT_EQ(std::string("1234567"), std::string(reader.get_buffer(7).data(), 7));
        reader.pop(7);
        read_bytes += 7;
        break;
      }
      EXPECT_EQ(std::string("12345678"), std::string(reader_buf.data(), 8));
      reader.pop(8);
      read_bytes += 8;
    }
    EXPECT_EQ(queue_size - 1, read_bytes);
  }

  // Now let's move the reader and the writer so that the next message is
  // wrapping around
  const std::string message("this_is_a_long_message");
  {
    queue_writer writer = queue_writer::queue_factory(queue_filepath, control_block_file);
    ASSERT_TRUE(writer.get_buffer(queue_size - message.size() / 2));
    writer.push(queue_size - message.size() / 2);

    ASSERT_TRUE(reader.get_buffer(queue_size - message.size() / 2));
    reader.pop(queue_size - message.size() / 2);

    queue_writer::mutable_view write_buf = writer.get_buffer(message.size());
    ASSERT_TRUE(write_buf);
    memcpy(write_buf.data(), message.data(), message.size());
    writer.push(message.size());
  }

  // Check that we can read correctly a string that is wrapping around:
  {
    queue_reader::const_view reader_buf = reader.get_buffer(message.size());
    ASSERT_TRUE(reader_buf);
    const std::string read_string(reader_buf.data(), message.size());
    EXPECT_EQ(message, read_string);
  }
}

TEST(Queue, queue_write_and_read_same_process)
{

  const std::string queue_filepath = "queue3.bin";
  file_cleaner cleaner(queue_filepath);

  constexpr size_t queue_size = 4096 * 10;
  file_descriptor queue_file(queue_filepath, queue_size);

  const std::string control_block_file = "control_block3.bin";
  file_cleaner cleaner_2(control_block_file);

  const std::string message("123456789");

  {
    queue_writer writer = queue_writer::queue_factory(queue_filepath, control_block_file);
    queue_writer::mutable_view write_buf = writer.get_buffer(message.size());
    ASSERT_TRUE(write_buf);
    memcpy(write_buf.data(), message.data(), message.size());
    writer.push(message.size());
  }

  {
    queue_reader reader = queue_reader::queue_factory(queue_filepath, control_block_file);

    queue_reader::const_view reader_buf = reader.get_buffer(message.size());
    ASSERT_TRUE(reader_buf);
    std::string_view read_message(reader_buf.data(), reader_buf.data() + message.size());
    EXPECT_EQ(message, read_message);
  }
}

TEST(Queue, writers_multiple_get_buffers)
{

  const std::string queue_filepath = "queue7.bin";
  file_cleaner cleaner(queue_filepath);

  constexpr size_t queue_size = 4096 * 10;
  file_descriptor queue_file(queue_filepath, queue_size);

  const std::string control_block_file = "control_block7.bin";
  file_cleaner cleaner_2(control_block_file);

  const std::string message_1("01234");
  const std::string message_2("56789");

  {
    queue_writer writer = queue_writer::queue_factory(queue_filepath, control_block_file);
    {
      queue_writer::mutable_view write_buf = writer.get_buffer(message_1.size());
      ASSERT_TRUE(write_buf);
      memcpy(write_buf.data(), message_1.data(), message_1.size());
    }
    {
      queue_writer::mutable_view write_buf = writer.get_buffer(message_2.size());
      ASSERT_TRUE(write_buf);
      memcpy(write_buf.data(), message_2.data(), message_2.size());
    }

    writer.push(message_1.size() + message_2.size());
  }

  {
    queue_reader reader = queue_reader::queue_factory(queue_filepath, control_block_file);

    const std::string message("0123456789");
    queue_reader::const_view reader_buf = reader.get_buffer(message.size());
    ASSERT_TRUE(reader_buf);
    std::string_view read_message(reader_buf.data(), reader_buf.data() + message.size());
    EXPECT_EQ(message, read_message);
  }
}

TEST(Queue, write_until_full)
{
  const std::string queue_filepath = "queue4.bin";
  file_cleaner cleaner(queue_filepath);

  constexpr size_t queue_size = 4096;
  file_descriptor queue_file(queue_filepath, queue_size);

  const std::string control_block_file = "control_block4.bin";
  file_cleaner cleaner_2(control_block_file);

  const std::string message("123456789");

  {
    queue_writer writer = queue_writer::queue_factory(queue_filepath, control_block_file);
    size_t bytes_written{};
    while (true)
    {
      queue_writer::mutable_view write_buf = writer.get_buffer(message.size());
      if (write_buf)
      {
        memcpy(write_buf.data(), message.data(), message.size());
        writer.push(message.size());
        bytes_written += message.size();
      }
      else
      {
        break;
      }
    }
    EXPECT_EQ(queue_size - (queue_size % message.size()), bytes_written);
  }

  // Now read a bit so that we can write more
  {
    queue_reader reader = queue_reader::queue_factory(queue_filepath, control_block_file);

    queue_reader::const_view reader_buf = reader.get_buffer(message.size());
    ASSERT_TRUE(reader_buf);
    std::string_view read_message(reader_buf.data(), reader_buf.data() + message.size());
    EXPECT_EQ(message, read_message);
    reader.pop(message.size());
  }

  {
    queue_writer writer = queue_writer::queue_factory(queue_filepath, control_block_file);
    queue_writer::mutable_view write_buf = writer.get_buffer(message.size());
    ASSERT_TRUE(write_buf);
  }
}

TEST(Queue, queue_write_and_read)
{
  const std::string queue_filepath = "queue6.bin";
  file_cleaner cleaner(queue_filepath);

  constexpr size_t queue_size = 4096;
  file_descriptor queue_file(queue_filepath, queue_size);

  const std::string control_block_file = "control_block6.bin";
  file_cleaner cleaner_2(control_block_file);

  const size_t num_insertions{1000000};
  const std::string message("0123456789");

  const int pid = fork();

  if (pid != 0)
  {
    // Parent
    queue_reader reader = queue_reader::queue_factory(queue_filepath, control_block_file);

    size_t bytes_read{};
    while (bytes_read != num_insertions * message.size())
    {
      queue_reader::const_view reader_buf = reader.get_buffer(message.size());
      if (reader_buf)
      {
        std::string_view read_message(reader_buf.data(), reader_buf.data() + message.size());
        EXPECT_EQ(message, read_message);
        reader.pop(message.size());
        bytes_read += message.size();
      }
    }
  }
  else
  {
    // Child
    queue_writer writer = queue_writer::queue_factory(queue_filepath, control_block_file);

    size_t bytes_written{};
    while (bytes_written != num_insertions * message.size())
    {
      queue_writer::mutable_view write_buf = writer.get_buffer(message.size());
      if (write_buf)
      {
        memcpy(write_buf.data(), message.data(), message.size());
        writer.push(message.size());
        bytes_written += message.size();
      }
    }
    exit(0);
  }
}

TEST(Queue, queue_write_and_read_random_messages)
{
  const std::string queue_filepath = "queue8.bin";
  file_cleaner cleaner(queue_filepath);

  constexpr size_t queue_size = 4096;
  file_descriptor queue_file(queue_filepath, queue_size);

  const std::string control_block_file = "control_block8.bin";
  file_cleaner cleaner_2(control_block_file);

  const size_t num_insertions{100'000};

  const int pid = fork();

  struct header
  {
    uint32_t size;
  } __attribute__((packed));

  if (pid != 0)
  {
    // Parent
    queue_reader reader = queue_reader::queue_factory(queue_filepath, control_block_file);

    int reads{};
    while (reads != num_insertions)
    {
      queue_reader::const_view reader_buf = reader.get_buffer(sizeof(header));
      if (reader_buf)
      {
        const header* const h = reinterpret_cast<const header*>(reader_buf.data());
        queue_reader::const_view message_buf = reader.get_buffer(h->size);
        ASSERT_TRUE(message_buf);

        std::string_view read_message(message_buf.data(),
                                      message_buf.data() + static_cast<size_t>(h->size));

        EXPECT_EQ('A', read_message[0]);
        EXPECT_EQ('B', read_message[1]);
        if (read_message[1] != 'B')
        {
          break;
        }
        reader.pop(sizeof(uint32_t) + h->size);
        reads++;
      }
    }
  }
  else
  {
    // Child
    queue_writer writer = queue_writer::queue_factory(queue_filepath, control_block_file);

    const std::string message_base = "A" + std::string(100, 'B');
    std::mt19937 gen(42);
    std::uniform_int_distribution<> distrib(2, message_base.size());

    int inserted{};
    while (inserted != num_insertions)
    {
      header h;

      h.size = distrib(gen);

      const size_t total_message_size = sizeof(h) + h.size;
      queue_writer::mutable_view write_buf = writer.get_buffer(total_message_size);
      if (write_buf)
      {
        std::memcpy(write_buf.data(), &h, sizeof(h));
        std::memcpy(write_buf.data() + sizeof(h), message_base.data(), h.size);

        writer.push(total_message_size);
        inserted++;
      }
    }
    exit(0);
  }
}