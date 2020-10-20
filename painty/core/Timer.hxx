/**
 * @file Timer.hxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-10-20
 *
 */
#pragma once

#include <chrono>
#include <functional>
#include <future>

namespace painty {

class Timer {
 public:
  Timer(bool singleShot);
  ~Timer();

  template <class Function, class... Arguments, class Rep, class Period>
  void start(const std::chrono::duration<Rep, Period>& timeout, Function&& f,
             Arguments&&... args) {
    std::function<typename std::result_of<Function(Arguments...)>::type()> task(
      std::bind(std::forward<Function>(f), std::forward<Arguments>(args)...));
    _thread = std::thread([&]() {
      do {
        std::this_thread::sleep_for(timeout);
        task();
      } while ((!_singleShot) && (!_terminate));
    });
  }

 private:
  std::thread _thread = {};
  std::atomic_bool _terminate;
  bool _singleShot;
};
}  // namespace painty
