#pragma once
#include "lockfree_spsc.h"
#include <deque>
#include <mutex>

/**
 * @brief This class will use a lockfree queue until a certain size, then it
 * will start using a locked queue. This is useful when we want the speed of
 * a lockfree queue, but we cannot put a hard limit on the size.
 *
 * @tparam T
 */
template <typename T>
class almost_always_lockfree_queue
{
public:
  explicit almost_always_lockfree_queue(size_t lockfree_size)
      : _base_queue(lockfree_size)
  {
  }

  void push(T&& item)
  {
    const bool using_extension =
        _extension_size.load(std::memory_order_acquire);

    // If we are not using the extension then try to push in the lockfree queue.
    if (!using_extension && _base_queue.try_push(std::forward<T>(item)))
    {
      return;
    }

    // We are either using the extension or we failed to push in the lockfree
    // queue. Time to push into the extension
    std::lock_guard l(_m);
    _extension.push_back(std::forward<T>(item));
    _extension_size.fetch_add(1, std::memory_order_release);

    return;
  }

  bool try_pop(T& popped)
  {
    // Try to pop from the lockfree queue, that's what gets filled first
    if (_base_queue.try_pop(popped))
    {
      return true;
    }

    // Failed to pop from the lockfree, try the extension.
    const bool using_extension =
        _extension_size.load(std::memory_order_acquire);

    if (using_extension)
    {
      std::lock_guard l(_m);
      popped = _extension.front();
      _extension.pop_front();
      _extension_size.fetch_sub(1, std::memory_order_release);
      return true;
    }

    return false;
  }

private:
  lockfree_spsc<T> _base_queue;
  std::deque<T> _extension;
  std::atomic_int _extension_size{};
  std::mutex _m;
};