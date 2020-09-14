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
#include "painty/renderer/TextureBrush.hxx"

TEST(TextureBrushTest, Construct) {
  auto brush = painty::TextureBrush<painty::vec3>(
    "data/sample_0");

  brush.dip({{{0.2, 0.3, 0.4}, {0.1, 0.23, 0.14}}});

  constexpr auto height = 500U;
  constexpr auto width  = 800;
  auto canvas           = painty::Canvas<painty::vec3>(height, width);
  canvas.clear();

  std::vector<painty::vec2> path;
  path.emplace_back(50.0, 250.0);
  path.emplace_back(400.0, 250.0);
  path.emplace_back(750.0, 250.0);

  brush.paintStroke(path, canvas);

  painty::Renderer<painty::vec3> renderer;

  painty::io::imSave("/tmp/canvasComposed.png", renderer.compose(canvas), true);
  painty::io::imSave("/tmp/getLightedRendering.png", renderer.render(canvas),
                     true);
}
