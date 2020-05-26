/**
 * @file canvas_test.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-15
 *
 */

#include <gtest/gtest.h>

#include <painty/texture_brush.h>
#include <painty/image_io.h>
#include <painty/renderer.h>

TEST(TextureBrushTest, Construct)
{
  auto brush = painty::TextureBrush<double, 3UL>("/home/tsl/development/painty/data/sample_0");

  brush.dip({ { { 0.2, 0.3, 0.4 }, { 0.1, 0.23, 0.14 } } });

  constexpr auto height = 500U;
  constexpr auto width = 800;
  auto canvas = painty::Canvas<double, 3UL>(height, width);
  canvas.clear();

  std::vector<painty::vec2> path;
  path.emplace_back(50.0, 250.0);
  path.emplace_back(400.0, 250.0);
  path.emplace_back(750.0, 250.0);

  brush.applyTo(path, canvas);

  painty::Renderer<double, 3UL> renderer;

  painty::io::imSave("/tmp/canvasComposed.png", renderer.compose(canvas));
  painty::io::imSave("/tmp/getLightedRendering.png", renderer.render(canvas));
}
