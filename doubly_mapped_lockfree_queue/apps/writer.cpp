#include "Logger.h"
#include "control_block.h"
#include "file_descriptor.h"
#include "message_header.h"
#include "queue_reader.h"
#include "queue_writer.h"
#include "simple_parser.hpp"

#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ratio>
#include <type_traits>
#include <x86intrin.h>

namespace
{
void signal_handler(int)
{
  Logger::Info("Exiting...");
  exit(0);
}

} // namespace

int main(int argc, const char* const argv[])
{
  simple_parser args;

  args.add_default("queue_file", "queue.bin");
  args.add_default("control_block_file", "control_block.bin");

  args.parse(argc, argv);

  const std::string queue_filepath = args.get_value("queue_file");
  const std::string control_block_file = args.get_value("control_block_file");

  {
    // This will create the queue, which could in principle already be there,
    // but for testing we remove the old one
    std::filesystem::remove(queue_filepath);
    std::filesystem::remove(control_block_file);

    const size_t queue_size = getpagesize() * 10;
    file_descriptor queue_file(queue_filepath, queue_size);
  }

  queue_writer writer = queue_writer::queue_factory(queue_filepath, control_block_file);

  signal(SIGINT, signal_handler);

  const std::string message("message");

  while (true)
  {
    {
      const uint64_t empty_spins = 100'000;
      const uint64_t start_tsc = __rdtsc();
      while (__rdtsc() - start_tsc < empty_spins)
      {
        // empty loop to simulate sparse events but keep the cpu hot
      };
    }

    const size_t total_message_size = sizeof(message_header) + message.size();

    queue_writer::mutable_view buffer = writer.get_buffer(total_message_size);
    if (buffer)
    {
      const uint64_t tsc = __rdtsc();
      message_header header{1, static_cast<uint32_t>(message.size()), tsc};

      std::memcpy(buffer.data(), &header, sizeof(header));
      std::memcpy(buffer.data() + sizeof(header), message.data(), message.size());

      writer.push(total_message_size);
    }
  }

  return 0;
}