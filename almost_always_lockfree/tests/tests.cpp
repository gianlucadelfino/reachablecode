#include <chrono>
#include <queue>
#include <thread>

#include "almost_always_lockfree_queue.h"
#include "gtest/gtest.h"

namespace
{
/**
 * @brief Returns true if the num is a multiple of "factor"
 */
inline bool is_multiple_of(size_t num, size_t factor) noexcept { return (num % factor) == 0; }

} // namespace

TEST(almost_always_lockfree_queue, push_and_pop)
{
  almost_always_lockfree_queue<int> q(5);

  {
    int _{};
    EXPECT_FALSE(q.try_pop(_));
  }

  for (size_t i = 0; i < 10; ++i)
  {
    q.push(i);
  }

  for (size_t i = 0; i < 10; ++i)
  {
    int popped{};
    EXPECT_TRUE(q.try_pop(popped));
    EXPECT_EQ(i, popped);
  }

  {
    int _{};
    EXPECT_FALSE(q.try_pop(_));
  }
}

TEST(almost_always_lockfree_queue, push_all_pop_exension_push_again)
{
  const size_t lockfree_size = 5;
  almost_always_lockfree_queue<int> q(lockfree_size);

  for (size_t i = 0; i < 10; ++i)
  {
    q.push(i);
  }

  for (size_t i = 0; i < lockfree_size; ++i)
  {
    int popped{};
    EXPECT_TRUE(q.try_pop(popped));
    EXPECT_EQ(i, popped);
  }

  for (size_t i = 10; i < 15; ++i)
  {
    q.push(i);
  }

  for (size_t i = lockfree_size; i < lockfree_size + 10; ++i)
  {
    int popped{};
    EXPECT_TRUE(q.try_pop(popped));
    EXPECT_EQ(i, popped);
  }

  {
    int _{};
    EXPECT_FALSE(q.try_pop(_));
  }
}

TEST(almost_always_lockfree_queue, concurrent_push_pop)
{
  const size_t lockfree_size = 10000;
  const size_t total_elems = 1000000;
  almost_always_lockfree_queue<int> q(lockfree_size);

  std::jthread t(
      [total_elems, &q]
      {
        for (size_t i = 0; i < total_elems; ++i)
        {
          q.push(i);
        }
      });

  for (size_t popped_num = 0; popped_num != total_elems;)
  {
    int popped{};
    if (q.try_pop(popped))
    {
      EXPECT_EQ(popped_num, popped);
      popped_num++;
    }
  }
}

TEST(almost_always_lockfree_queue, concurrent_push_pop_slow_writer)
{
  const size_t lockfree_size = 10000;
  const size_t total_elems = 1000000;
  almost_always_lockfree_queue<int> q(lockfree_size);

  std::jthread t(
      [total_elems, &q]
      {
        for (size_t i = 0; i < total_elems; ++i)
        {
          q.push(i);
          if (is_multiple_of(i, 10000))
          {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }
        }
      });

  for (size_t popped_num = 0; popped_num != total_elems;)
  {
    int popped{};
    if (q.try_pop(popped))
    {
      EXPECT_EQ(popped_num, popped);
      popped_num++;
    }
  }
}

TEST(almost_always_lockfree_queue, concurrent_push_pop_slow_reader)
{
  const size_t lockfree_size = 10000;
  const size_t total_elems = 1000000;
  almost_always_lockfree_queue<int> q(lockfree_size);

  std::jthread t(
      [total_elems, &q]
      {
        for (size_t i = 0; i < total_elems; ++i)
        {
          q.push(i);
        }
      });

  for (size_t popped_num = 0; popped_num != total_elems;)
  {
    int popped{};
    if (q.try_pop(popped))
    {
      if (is_multiple_of(popped_num, 10000))
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }

      EXPECT_EQ(popped_num, popped);
      popped_num++;
    }
  }
}
