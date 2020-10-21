/**
 * @file ThreadPool.hxx
 * @author thomas lindemeier
 * @brief
 * @date 2020-10-20
 *
 */
#pragma once
#include <functional>
#include <future>
#include <queue>
#include <thread>

namespace painty {

class ThreadPool final {
 private:
  template <class Function, class... Args>
  std::future<typename std::result_of<Function(Args...)>::type> push(
    bool back, Function&& f, Args&&... args) {
    std::packaged_task<typename std::result_of<Function(Args...)>::type()> task(
      std::bind(f, args...));
    auto res = task.get_future();
    {
      std::unique_lock<std::mutex> lock(_mutex);
      if (back) {
        _tasks.push_back(std::packaged_task<void()>(std::move(task)));
      } else {
        _tasks.push_front(std::packaged_task<void()>(std::move(task)));
      }
    }
    _condition.notify_one();

    return res;
  }

  void initWorkers();

  /**
   * @brief Start the thread pool.
   *
   */
  void start();
  /**
   * @brief Terminate the thread pool.
   * This gets automatically called if this object gets out of scope.
   *
   */
  void stop();

 public:
  ThreadPool(std::size_t threadCount);
  ~ThreadPool();
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  /**
   * @brief Remove all jobs that are queued.
   *
   */
  void clear();

  template <class Function, class... Args>
  std::future<typename std::result_of<Function(Args...)>::type> add_front(
    Function&& f, Args&&... args) {
    return push(false, f, args...);
  }

  template <class Function, class... Args>
  std::future<typename std::result_of<Function(Args...)>::type> add_back(
    Function&& f, Args&&... args) {
    return push(true, f, args...);
  }

 private:
  std::vector<std::thread> _threads;
  std::mutex _mutex;
  std::condition_variable _condition;
  std::atomic_bool _stop;

  std::deque<std::packaged_task<void()> > _tasks;
};

}  // namespace painty
