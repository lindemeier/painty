/**
 * @file paint_layer_test.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-14
 *
 */

#include <gtest/gtest.h>
#include <painty/renderer/paint_layer.h>

TEST(PaintLayerTest, Construct) {
  auto layer = painty::PaintLayer<painty::vec3>(800, 600);
  EXPECT_EQ(layer.getCols(), layer.getK_buffer().cols);
  EXPECT_EQ(layer.getCols(), layer.getS_buffer().cols);
  EXPECT_EQ(layer.getCols(), layer.getV_buffer().cols);
  EXPECT_EQ(layer.getRows(), layer.getK_buffer().rows);
  EXPECT_EQ(layer.getRows(), layer.getS_buffer().rows);
  EXPECT_EQ(layer.getRows(), layer.getV_buffer().rows);

  layer.clear();

  auto& K            = layer.getK_buffer();
  auto& S            = layer.getS_buffer();
  auto& V            = layer.getV_buffer();
  constexpr auto Eps = std::numeric_limits<double>::epsilon();
  for (auto i = 0; i < static_cast<int32_t>(K.total()); i++) {
    for (auto j = 0; j < K(i).rows(); j++) {
      EXPECT_NEAR(K(i)[j], 0.0, Eps);
    }
    for (auto j = 0; j < S(i).rows(); j++) {
      EXPECT_NEAR(S(i)[j], 0.0, Eps);
    }
    EXPECT_NEAR(V(i), 0.0, Eps);
  }

  auto r0 = painty::Mat<painty::vec<double, 3U>>(800, 600);
  layer.composeOnto(r0);

  auto p_other = painty::PaintLayer<painty::vec3>(800, 600);
  layer.copyTo(p_other);
}
