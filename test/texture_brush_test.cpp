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

TEST(TextureBrushTest, Construct)
{
  auto brush = painty::TextureBrush<double, 3UL>("/home/tsl/development/painty/data/sample_0");

  brush.dip({ { { 0.2, 0.3, 0.4 }, { 0.1, 0.23, 0.14 } } });

  auto canvas = painty::Canvas<double, 3UL>(500, 800);
  canvas.clear();

  brush.applyTo({ { 10.0, 10.0 }, { 50.0, 50.0 }, { 150.0, 150.0 }, { 300.0, 300.0 } }, canvas);

  // const auto linearRGB = canvas.getLightedRendering();
  const auto linearRGB = canvas.composed();
  painty::io::imSave("/tmp/testRender.png", linearRGB);
}
