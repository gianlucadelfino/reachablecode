#include <atomic>
#include <cassert>
#include <chrono>
#include <exception>
#include <future>
#include <iostream>
#include <list>
#include <mutex>
#include <optional>
#include <thread>
#include <concepts>

class simple_thread_pool
{
  public:
  using Task = std::packaged_task<void(void)>;
  explicit simple_thread_pool(
      size_t num_threads_ = std::thread::hardware_concurrency())
      : _running(true)
  {
    assert(num_threads_);
    for (size_t i = 0; i < num_threads_; ++i)
    {
      _threads.emplace_back(std::thread([this] { this->thread_loop(); }));
    }
  }

  simple_thread_pool(simple_thread_pool&&) = delete;
  simple_thread_pool(const simple_thread_pool&) = delete;
  simple_thread_pool& operator=(simple_thread_pool&&) = delete;
  simple_thread_pool& operator=(const simple_thread_pool&) = delete;

  ~simple_thread_pool()
  {
    try
    {
      sync_stop();
    }
    catch (const std::exception& e_)
    {
      std::cout << "Exception on destruction: " << e_.what() << std::endl;
    }
  }

  template <std::regular_invocable Callable,
            typename return_type = typename std::result_of<Callable()>::type>
  std::future<return_type> add_task2(Callable&& callable_)
  {
    std::promise<return_type> p;
    std::future<return_type> f = p.get_future();

    std::packaged_task<void(void)> task(
        [p = std::move(p), c = std::move(callable_)]() mutable {
          try
          {
            if constexpr (std::is_same<return_type, void>::value)
            {
              c();
              p.set_value();
            }
            else
            {
              p.set_value(c());
            }
          }
          catch (...)
          {
            p.set_exception(std::current_exception());
          }
        });

    std::lock_guard<std::mutex> l(_m);
    unsafe_add_task(std::move(task));

    return f;
  }

  void add_task(Task&& task_)
  {
    std::lock_guard l(_m);
    unsafe_add_task(std::move(task_));
  }

  /**
   * Blocks until all the threads complete what they are doing and stop
   * @throw std::runtime_error if any of the tasks threw an error
   */
  void sync_stop()
  {
    if (!_running)
    {
      return;
    }

    Task stop_task([this] { _running = false; });

    add_task(std::move(stop_task));

    for (auto& t : _threads)
    {
      if (t.joinable())
      {
        t.join();
      }
    }
  }

  private:
  void thread_loop()
  {
    while (_running)
    {
      std::optional<Task> t = pop_task();

      if (t)
      {
        assert(t->valid());

        // Now run the task.
        t->operator()();
      }
      else
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
      }
    }
  }

  std::optional<Task> pop_task()
  {
    std::lock_guard<std::mutex> l(_m);
    std::optional<Task> t;
    if (_tasks.empty())
    {
      return t;
    }
    else
    {

      t = std::move(_tasks.back());
      _tasks.pop_back();
      return t;
    }
  }

  void unsafe_add_task(Task&& task_)
  {
    assert(task_.valid());
    _tasks.emplace_front(std::move(task_));
  }

  std::mutex _m;
  std::atomic_bool _running{};
  std::list<Task> _tasks;

  std::list<std::thread> _threads;
  std::exception_ptr _exception{};
};
