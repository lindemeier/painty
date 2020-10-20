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

class ThreadPool {
 private:
  std::thread _thread;
  std::mutex _mutex;
  std::condition_variable _condition;
  std::atomic_bool _terminate;
  std::atomic_bool _executing;

  std::deque<std::packaged_task<void()> > _tasks;

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

  void startWorker();

 public:
  ThreadPool();
  virtual ~ThreadPool();

  void start();      // continue executing jobs
  void stop();       // pause executing jobs
  void terminate();  // shut down thread
  void clear();      // clear current tasks

  template <class Function, class... Args>
  std::future<typename std::result_of<Function(Args...)>::type> push_front(
    Function&& f, Args&&... args) {
    return push(false, f, args...);
  }

  template <class Function, class... Args>
  std::future<typename std::result_of<Function(Args...)>::type> push_back(
    Function&& f, Args&&... args) {
    return push(true, f, args...);
  }
};

}  // namespace painty
