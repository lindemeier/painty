/**
 * @file ThreadPool.cxx
 * @author thomas lindemeier
 * @brief
 * @date 2020-10-20
 *
 */

#include "painty/core/ThreadPool.hxx"

namespace painty {

ThreadPool::ThreadPool()
    : _mutex(),
      _condition(),
      _terminate(false),
      _executing(true),
      _tasks() {
  startWorker();
}

ThreadPool::~ThreadPool() {
  terminate();
}

void ThreadPool::start() {
  if (!_executing) {
    _executing = true;

    _condition.notify_all();
  }
}

void ThreadPool::stop() {
  _executing = false;
}

void ThreadPool::terminate() {
  _terminate = true;

  _condition.notify_all();

  if (_thread.joinable())
    _thread.join();
}

void ThreadPool::clear() {
  std::unique_lock<std::mutex> lock(_mutex);
  _tasks.clear();
}

void ThreadPool::startWorker() {
  _terminate = false;

  _thread = std::thread([&]() {
    while (!_terminate) {
      std::packaged_task<void()> f;

      std::unique_lock<std::mutex> lock(_mutex);

      while (_tasks.empty() || !_executing) {
        _condition.wait(lock);
        if (_terminate) {
          return;
        }
      }

      f = std::move(_tasks.front());
      _tasks.pop_front();

      lock.unlock();

      f();
    }
  });
}

}  // namespace painty
