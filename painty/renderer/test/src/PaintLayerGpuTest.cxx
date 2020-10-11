/**
 * @file PaintLayerGpuTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-10-1
 *
 */

#include "gtest/gtest.h"
#include "painty/gpu/GpuMat.hxx"
#include "painty/io/ImageIO.hxx"
#include "painty/renderer/PaintLayerGpu.hxx"
#include "prgl/Window.hxx"

TEST(PaintLayerGpuTest, Construct) {
  const auto window = std::make_unique<prgl::Window>(800, 600, "window", false);

  auto layer = painty::PaintLayerGpu({800U, 600U});

  painty::Mat<painty::vec3f> R0(600, 800);
  for (auto& a : R0) {
    a = {0.0F, 1.0F, 0.0F};
  }
  auto tex_R0 = painty::GpuMat<painty::vec3f>(R0);

  layer.composeOnto(tex_R0);

  tex_R0.download();

  painty::io::imSave("/tmp/rgb.png", tex_R0.getMat(), false);
}
