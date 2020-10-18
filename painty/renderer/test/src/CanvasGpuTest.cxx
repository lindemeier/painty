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
#include "painty/renderer/CanvasGpu.hxx"
#include "painty/renderer/TextureBrushGpu.hxx"
#include "prgl/Window.hxx"

TEST(CanvasGpuTest, Construct) {
  constexpr auto height = 768U;
  constexpr auto width  = 1024U;

  const auto window =
    std::make_shared<prgl::Window>(1024U, 768U, "window", false);

  auto brush = painty::TextureBrushGpu(window);

  brush.dip({{{0.2, 0.3, 0.4}, {0.1, 0.23, 0.14}}});

  auto canvas           = painty::CanvasGpu({width, height});
  canvas.clear();

  std::vector<painty::vec2> path;
  path.emplace_back(50.0, 250.0);
  path.emplace_back(400.0, 250.0);
  path.emplace_back(650.0, 250.0);
  brush.setRadius(40.0);

  brush.paintStroke(path, canvas);
}
