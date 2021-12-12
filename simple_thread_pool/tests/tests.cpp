#include <algorithm>
#include <future>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "simple_thread_pool.hpp"

TEST(simple_thread_pool, no_tasks) { simple_thread_pool pool; }

TEST(simple_thread_pool, one_task)
{
  simple_thread_pool pool(3);
  std::packaged_task<void(void)> task([]() { ASSERT_TRUE(true); });
  pool.add_task(std::move(task));
}

TEST(simple_thread_pool, one_task_wait)
{
  simple_thread_pool pool(3);
  std::packaged_task<void(void)> task([]() { ASSERT_TRUE(true); });
  std::future<void> task_fut = task.get_future();

  pool.add_task(std::move(task));
  task_fut.get();
}

TEST(simple_thread_pool, one_task_except)
{
  simple_thread_pool pool(3);
  std::packaged_task<void(void)> task(
      []() { throw std::runtime_error("ERROR!"); });

  pool.add_task(std::move(task));
}

TEST(simple_thread_pool, one_task2_void)
{
  simple_thread_pool pool(3);
  auto fut = pool.add_task2([]() { ASSERT_TRUE(true); });

  fut.get();
}

TEST(simple_thread_pool, one_task2_int)
{
  simple_thread_pool pool(3);
  auto fut = pool.add_task2([]() -> int { return 1; });

  EXPECT_EQ(1, fut.get());
}

TEST(simple_thread_pool, one_task2_void_int)
{
  simple_thread_pool pool(3);
  auto fut = pool.add_task2([]() -> int { return 1 + 2; });

  auto fut2 = pool.add_task2([]() { ASSERT_TRUE(true); });

  fut2.get();

  EXPECT_EQ(3, fut.get());
}

TEST(simple_thread_pool, one_task2_except)
{
  simple_thread_pool pool(3);
  auto fut = pool.add_task2([]() { throw std::runtime_error("ERROR!"); });
  EXPECT_THROW(fut.get(), std::runtime_error);
}
