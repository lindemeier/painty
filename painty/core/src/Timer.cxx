/**
 * @file Timer.cxx
 * @author thomas lindemeier
 * @brief
 * @date 2020-10-20
 *
 */

#include "painty/core/Timer.hxx"

namespace painty {

Timer::Timer(bool singleShot) : _terminate(false), _singleShot(singleShot) {}

Timer::~Timer() {
  _terminate = true;

  if (_thread.joinable()) {
    _thread.join();
  }
}

}  // namespace painty
