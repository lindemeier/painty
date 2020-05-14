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

#include <painty/paint_layer.h>

TEST(PaintLayerTest, Construct)
{
  auto layer = painty::PaintLayer<double, 3UL>(800, 600);
  EXPECT_EQ(layer.getCols(), layer.getK_buffer().getCols());
  EXPECT_EQ(layer.getCols(), layer.getS_buffer().getCols());
  EXPECT_EQ(layer.getCols(), layer.getV_buffer().getCols());
  EXPECT_EQ(layer.getRows(), layer.getK_buffer().getRows());
  EXPECT_EQ(layer.getRows(), layer.getS_buffer().getRows());
  EXPECT_EQ(layer.getRows(), layer.getV_buffer().getRows());

  layer.clear();

  auto& K = layer.getK_buffer().getData();
  auto& S = layer.getS_buffer().getData();
  auto& V = layer.getV_buffer().getData();
  constexpr auto Eps = std::numeric_limits<double>::epsilon();
  for (size_t i = 0; i < K.size(); i++)
  {
    for (auto v : K[i])
    {
      EXPECT_NEAR(v, 0.0, Eps);
    }
    for (auto v : S[i])
    {
      EXPECT_NEAR(v, 0.0, Eps);
    }
    EXPECT_NEAR(V[i], 0.0, Eps);
  }

  auto r0 = painty::Mat<painty::vec<double, 3U>>(800, 600);
  layer.composeOnto(r0);

  auto p_other = painty::PaintLayer<double, 3UL>(800, 600);
  layer.copyTo(p_other);
}
