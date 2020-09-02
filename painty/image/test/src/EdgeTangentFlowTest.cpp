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
    const painty::vec2 v0_expected = {0.80594522703129268,
                                      -0.59199011058080875};
    EXPECT_NEAR(v0[0], v0_expected[0], Eps);
    EXPECT_NEAR(v0[1], v0_expected[1], Eps);
  }
  {
    const auto v0                  = etf(651, 68);
    const painty::vec2 v0_expected = {0.68232404693122051,
                                      -0.73104985806674061};
    EXPECT_NEAR(v0[0], v0_expected[0], Eps);
    EXPECT_NEAR(v0[1], v0_expected[1], Eps);
  }
  {
    const auto v0                  = etf(700, 2);
    const painty::vec2 v0_expected = {0.96539425013213997,
                                      -0.26079482704187806};
    EXPECT_NEAR(v0[0], v0_expected[0], Eps);
    EXPECT_NEAR(v0[1], v0_expected[1], Eps);
  }
  {
    const auto v0                  = etf(800, 134);
    const painty::vec2 v0_expected = {33.965732810242017, -23.807639308987106};
    EXPECT_NEAR(v0[0], v0_expected[0], Eps);
    EXPECT_NEAR(v0[1], v0_expected[1], Eps);
  }
  // const auto vis = painty::lineIntegralConv(etf, 80.0);
}
