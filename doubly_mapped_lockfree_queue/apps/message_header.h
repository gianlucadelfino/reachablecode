#pragma once

#include <cstdint>
#include <type_traits>

#pragma pack(push, 1)
/**
 * @brief This is an example of a typical message header. The user can customize
 * this at will, but it should still satisfy the static_assert
 */
struct message_header
{
  uint32_t version;
  uint32_t size;
  uint64_t timestamp;
};
#pragma pack(pop)

static_assert(std::is_trivial<message_header>::value &&
                  std::is_standard_layout<message_header>::value &&
                  std::alignment_of<message_header>::value == 1,
              "The header should be trivial");
