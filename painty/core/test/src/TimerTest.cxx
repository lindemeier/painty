/**
 * @file TimerTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-10-20
 *
 */

#include "gtest/gtest.h"
#include "painty/core/Timer.hxx"

TEST(TimerTestTest, Construct) {
  painty::Timer timer(false);

  timer.start(std::chrono::milliseconds(1000U), [=]() {

  });
}
