#pragma once

#include <chrono>

namespace
{
/**
 * @brief The CountdownTimer class can be used to check if a predetermined
 * amout of time has passed
 */
class CountdownTimer
{
  public:
  explicit CountdownTimer(std::chrono::milliseconds millis_)
      : _timeout(millis_), _start(std::chrono::system_clock::now())
  {
  }

  bool is_it_time_yet()
  {
    const auto now = std::chrono::system_clock::now();
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - _start);
    const bool it_is_time = elapsed >= _timeout;
    if (it_is_time)
    {
      _start = now;
    }
    return it_is_time;
  }

  private:
  const std::chrono::milliseconds _timeout;
  std::chrono::system_clock::time_point _start;
};

} // namespace
