#include <iostream>

#include "simple_thread_pool.hpp"

int main()
{
  simple_thread_pool pool(3);
  auto fut = pool.add_task2([]() -> int { return 1 + 2; });

  fut.get();
}