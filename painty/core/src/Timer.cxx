/**
 * @file Timer.cxx
 * @author thomas lindemeier
 * @brief
 * @date 2020-10-20
 *
 */

#include "painty/core/Timer.hxx"

namespace painty {

Timer::Timer() : _stop(false) {}

Timer::~Timer() {
  _stop = true;

  if (_future.valid()) {
    _future.wait();
  }
}

}  // namespace painty
