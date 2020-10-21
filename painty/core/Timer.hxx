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
/**
 * @brief A timer object that can be used to call a function at specific time intervals.
 *
 */
class Timer {
 public:
  /**
  * @brief Construct a new Timer object
  *
  * @param singleShot if true, the timer calls the function once and then releases the thread.
  */
  Timer(bool singleShot);
  ~Timer();

  /**
   * @brief
   *
   * @tparam Function The type of the function to call.
   * @tparam Arguments The types of the arguments passed to that function.
   * @param timeout The time interval.
   * @param f The function to call.
   * @param args the arguments of the function.
   */
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

    if (_thread.joinable()) {
      _thread.join();
    }
  }

 private:
  std::thread _thread = {};
  std::atomic_bool _terminate;
  bool _singleShot;
};
}  // namespace painty
