#include <map>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "simple_parser.hpp"

TEST(simple_parser, many_paramenters)
{
  const char* const parameters[5]{
      "name_of_program", "--param1", "value1", "--param2", "value2"};

  simple_parser args(5, parameters);

  std::map<std::string, std::string> expected_parameters{{"param1", "value1"},
                                                         {"param2", "value2"}};

  for (const auto& [key, value] : args)
  {
    EXPECT_EQ(expected_parameters[key], value);
  }
}
