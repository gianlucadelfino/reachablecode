#include "Logger.h"
#include "Math.h"
#include "TimeLogger.h"

#include "control_block.h"
#include "file_descriptor.h"
#include "message_header.h"
#include "queue_reader.h"
#include "queue_writer.h"
#include "simple_parser.hpp"

#include <algorithm>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sys/types.h>
#include <vector>
#include <x86intrin.h> // __rdtsc()

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
  args.add_default("num_messages", 100'000);

  args.parse(argc, argv);

  const std::string queue_filepath = args.get_value("queue_file");
  const std::string control_block_file = args.get_value("control_block_file");
  const int num_messages = args.get_value<int>("num_messages");

  {
    // This will create the queue if it wasn't there, and adjust its size
    const size_t queue_size = getpagesize() * 10;
    file_descriptor queue_file(queue_filepath, queue_size);
  }

  queue_reader reader = queue_reader::queue_factory(queue_filepath, control_block_file);

  signal(SIGINT, signal_handler);

  const size_t num_samples = num_messages;
  size_t warmup = 10'000;

  std::vector<uint64_t> deltas;
  deltas.reserve(num_samples);

  {
    time_logger timer("Receiving " + std::to_string(num_messages) + " messages", std::cout);

    while (true)
    {
      const queue_reader::const_view header_buffer = reader.get_buffer(sizeof(message_header));
      if (header_buffer)
      {
        const message_header* const header =
            reinterpret_cast<const message_header*>(header_buffer.data());
        const queue_reader::const_view message_buffer = reader.get_buffer(header->size);
        assert(message_buffer);

        const uint64_t tsc = __rdtsc();
        const uint64_t delta_tsc = tsc - header->timestamp;

        if (deltas.size() < num_samples)
        {
          if (warmup > 0)
          {
            warmup--;
          }
          else
          {
            deltas.push_back(delta_tsc);
          }
        }
        else
        {
          break;
        }
        std::string_view read_message(message_buffer.data(), message_buffer.data() + header->size);

        (void)read_message;
        // Logger::Info("Read:", read_message, ", timestamp", header->timestamp, ", delta:",
        // delta_tsc);

        reader.pop(header_buffer.size() + message_buffer.size());
      }
    }

  } // timer stop

  std::filesystem::remove("deltas.dat");
  std::ofstream deltas_out("deltas.dat");
  for (auto& d : deltas)
  {
    deltas_out << d << std::endl;
  }

  // Output stats
  const double average = math::average(deltas.cbegin(), deltas.cend());
  const double st_dev = math::standard_deviation(deltas.cbegin(), deltas.cend(), average);
  const double ninety_nine_percentile =
      math::percentile(deltas.begin(), deltas.end(), 99); // partial sort

  Logger::Info("Average:", average);
  Logger::Info("st. dev:", st_dev);
  Logger::Info("99th percentile:", ninety_nine_percentile);
  return 0;
}