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
#include <painty/canvas.h>

TEST(CanvasTest, Construct) {
  auto layer = painty::Canvas<painty::vec3>(800, 600);

  layer.clear();

  layer.getR0();
  layer.get_h();
  layer.getReflectanceLayerDry();

  layer.setBackground(painty::Mat<painty::vec<double, 3UL>>(800, 600));

  layer.getPaintLayer();
  layer.getTimeMap();
  layer.setDryingTime(std::chrono::milliseconds(5000));
  layer.checkDry(5U, 5U, std::chrono::system_clock::now());
}
