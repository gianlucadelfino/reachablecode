#include <iostream>

#include "simple_parser.hpp"

int main(int argc, const char * const argv[])
{
  simple_parser args(argc, argv);

  args.add_default("my_param", "hello");

  for (const auto& [key, value] : args)
  {
    std::cout << "Key " << key << ", value " << value << std::endl;
  }
}