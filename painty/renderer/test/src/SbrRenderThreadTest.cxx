/**
 * @file SbrRenderThreadTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-10-21
 *
 */

#include "gtest/gtest.h"
#include "painty/gpu/GpuTaskQueue.hxx"
#include "painty/renderer/SbrRenderThread.hxx"

TEST(SbrRenderThreadTest, Construct) {
  const auto windowSize = painty::Size{1024U, 768U};
  const auto queue      = std::make_shared<painty::GpuTaskQueue>(windowSize);
  painty::SbrRenderThread renderThread(queue, windowSize);
}
