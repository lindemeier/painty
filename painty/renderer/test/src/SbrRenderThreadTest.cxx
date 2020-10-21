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
#include "painty/renderer/SbrRenderThread.hxx"

TEST(SbrRenderThreadTest, Construct) {
  painty::SbrRenderThread renderThread({1024U, 768U});
}
