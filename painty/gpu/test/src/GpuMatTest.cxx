/**
 * @file GpuMatTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-10-08
 *
 */
#include "gtest/gtest.h"
#include "painty/gpu/GpuMat.hxx"
#include "prgl/Window.hxx"

TEST(GpuMatTest, Construct) {
  std::make_shared<prgl::Window>(1024, 768, "window", true);

  painty::Mat1f cpuMat(768, 1024);
  {
    uint32_t i = 0U;
    for (auto& a : cpuMat) {
      a = static_cast<float>(i++) / static_cast<float>(cpuMat.total());
    }
  }

  auto cpuMatClone = cpuMat.clone();
  auto gpuMat      = painty::GpuMat<float>(cpuMatClone);
  gpuMat.download();

  const auto cpuMatRead = gpuMat.getMat();

  EXPECT_EQ(cpuMatRead.size(), cpuMatClone.size());

  for (auto l = 0; l < static_cast<int32_t>(cpuMatClone.total()); l++) {
    EXPECT_NEAR(cpuMatRead(l), cpuMatClone(l), 0.00001F);
  }
}
