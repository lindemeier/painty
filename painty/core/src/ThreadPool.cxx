/**
 * @file ThreadPool.cxx
 * @author thomas lindemeier
 * @brief
 * @date 2020-10-20
 *
 */

#include "painty/core/ThreadPool.hxx"

namespace painty {

ThreadPool::ThreadPool(std::size_t threadCount)
    : _threads(threadCount),
      _mutex(),
      _condition(),
      _stop(false),
      _tasks() {
  initWorkers();
  start();
}

ThreadPool::~ThreadPool() {
  stop();
}

void ThreadPool::start() {
  _condition.notify_all();
}

void ThreadPool::stop() {
  _stop = true;

  _condition.notify_all();

  for (auto& t : _threads) {
    if (t.joinable()) {
      t.join();
    }
  }
}

void ThreadPool::clear() {
  std::unique_lock<std::mutex> lock(_mutex);
  _tasks.clear();
}

void ThreadPool::initWorkers() {
  _stop = false;
  for (auto& thread : _threads) {
    thread = std::thread([&]() {
      while (!_stop) {
        std::packaged_task<void()> f;

        std::unique_lock<std::mutex> lock(_mutex);

        while (_tasks.empty()) {
          _condition.wait(lock);
          if (_stop) {
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
}

}  // namespace painty
