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

  painty::Mat<painty::vec4f> R0(600, 800);
  for (auto& a : R0) {
    a = {0.0F, 1.0F, 0.0F, 0.0F};
  }
  auto tex_R0 = painty::GpuMat<painty::vec4f>(R0);

  layer.composeOnto(tex_R0);

  tex_R0.download();

  const auto m = tex_R0.getMat();
  painty::Mat<painty::vec3f> R0_3(m.size());
  for (auto i = 0; i < static_cast<int32_t>(m.total()); i++) {
    R0_3(i) = {m(i)[0U], m(i)[1U], m(i)[2U]};
  }

  painty::io::imSave("/tmp/rgb.png", R0_3, false);
}
