/**
 * @file EdgeTangentFlowTest.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-07-31
 *
 */
#include "gtest/gtest.h"
#include "painty/core/Color.h"
#include "painty/image/EdgeTangentFlow.h"
#include "painty/io/ImageIO.h"

TEST(MatTest, EdgeTangentFlowTest) {
  painty::Mat3d image;
  // load an image as linear rgb
  painty::io::imRead("./data/test_images/field.jpg", image, true);
  // convert to Lab color space
  painty::Mat3d labImage(image.size());
  painty::ColorConverter<double> converter;
  painty::transform(image.begin(), image.end(), labImage.begin(),
                    [&converter](const painty::vec3& rgb, painty::vec3& lab) {
                      converter.rgb2lab(rgb, lab);
                    });

  // compute tensors
  const auto tensors =
    painty::tensor::ComputeTensors(labImage, painty::Mat1d(), 0.0, 0.5);
  const auto etf = painty::ComputeEdgeTangentFlow(tensors);

  constexpr auto Eps = 0.000001;

  {
    const auto v0                  = etf(123, 98);
    const painty::vec2 v0_expected = {-0.99992, -0.0126501};
    EXPECT_NEAR(v0[0], v0_expected[0], Eps);
    EXPECT_NEAR(v0[1], v0_expected[1], Eps);
  }
  {
    const auto v0                  = etf(651, 68);
    const painty::vec2 v0_expected = {0.83832, -0.545178};
    EXPECT_NEAR(v0[0], v0_expected[0], Eps);
    EXPECT_NEAR(v0[1], v0_expected[1], Eps);
  }
  {
    const auto v0                  = etf(700, 2);
    const painty::vec2 v0_expected = {0.915475, -0.402374};
    EXPECT_NEAR(v0[0], v0_expected[0], Eps);
    EXPECT_NEAR(v0[1], v0_expected[1], Eps);
  }
  {
    const auto v0                  = etf(800, 134);
    const painty::vec2 v0_expected = {0.107728, 0.0384994};
    EXPECT_NEAR(v0[0], v0_expected[0], Eps);
    EXPECT_NEAR(v0[1], v0_expected[1], Eps);
  }
  // const auto vis = painty::lineIntegralConv(etf, 80.0);
}
