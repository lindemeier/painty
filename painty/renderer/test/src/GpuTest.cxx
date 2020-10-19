/**
 * @file CanvasGpuTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-10-15
 *
 */

#include "gtest/gtest.h"
#include "painty/gpu/GpuMat.hxx"
#include "painty/io/ImageIO.hxx"
#include "painty/renderer/CanvasGpu.hxx"
#include "painty/renderer/TextureBrushGpu.hxx"
#include "prgl/Window.hxx"

constexpr auto height = 768U;
constexpr auto width  = 1024U;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
const auto window =
  std::make_shared<prgl::Window>(width, height, "window", false);
#pragma clang diagnostic pop

TEST(GpuMatTest, Construct) {
  painty::GpuMat<painty::vec4f> m(painty::Size{1024U, 768U});

  m.download();

  const auto host_m = m.getMat();
}

TEST(CanvasGpuTest, Construct) {
  auto brush = painty::TextureBrushGpu(window);

  brush.dip({{{0.2, 0.3, 0.4}, {0.1, 0.23, 0.14}}});

  auto canvas = painty::CanvasGpu({width, height});
  canvas.clear();

  std::vector<painty::vec2> path;
  path.emplace_back(50.0, 250.0);
  path.emplace_back(400.0, 250.0);
  path.emplace_back(650.0, 250.0);
  brush.setRadius(40.0);

  brush.paintStroke(path, canvas);
}

TEST(PaintLayerGpuTest, Construct) {
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

TEST(TextureBrushGpuTest, Construct) {
  auto brush = painty::TextureBrushGpu(window);

  brush.dip({{{0.2, 0.3, 0.4}, {0.1, 0.23, 0.14}}});

  auto canvas = painty::CanvasGpu({width, height});
  canvas.clear();

  std::vector<painty::vec2> path;
  path.emplace_back(50.0, 250.0);
  path.emplace_back(400.0, 250.0);
  path.emplace_back(650.0, 250.0);
  brush.setRadius(40.0);

  brush.paintStroke(path, canvas);

  const auto rgb = canvas.getComposition();

  painty::io::imSave("/tmp/canvasGpuComposed.png", rgb, true);
}
