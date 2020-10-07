/**
 * @file CanvasTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-15
 *
 */

#include "gtest/gtest.h"
#include "painty/io/ImageIO.hxx"
#include "painty/renderer/Renderer.hxx"
#include "painty/renderer/TextureBrushGpu.hxx"
#include "prgl/Window.hxx"

TEST(TextureBrushGpuTest, Construct) {
  auto brush = painty::TextureBrushGpu(
    std::make_shared<prgl::Window>(1024, 768, "window", true));

  brush.dip({{{0.2, 0.3, 0.4}, {0.1, 0.23, 0.14}}});

  constexpr auto height = 768;
  constexpr auto width  = 1024;
  auto canvas           = painty::Canvas<painty::vec3>(height, width);
  canvas.clear();

  std::vector<painty::vec2> path;
  path.emplace_back(50.0, 250.0);
  path.emplace_back(400.0, 250.0);
  path.emplace_back(650.0, 250.0);
  brush.setRadius(40.0);

  brush.paintStroke(path, canvas);
}
