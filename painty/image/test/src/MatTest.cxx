/**
 * @file MatTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */
#include "gtest/gtest.h"
#include "painty/image/Mat.hxx"

TEST(MatTest, Construct) {
  painty::Mat<double> m0;

  EXPECT_TRUE(m0.empty());

  EXPECT_EQ(0U, m0.cols);
  EXPECT_EQ(0U, m0.rows);

  m0 = painty::Mat<double>(2U, 4U);
  EXPECT_FALSE(m0.empty());
  EXPECT_EQ(4U, m0.cols);
  EXPECT_EQ(2U, m0.rows);
}

TEST(MatTest, Assign) {
  painty::Mat<double> m0 = painty::Mat<double>(2U, 4U);

  EXPECT_FALSE(m0.empty());
  EXPECT_EQ(4U, m0.cols);
  EXPECT_EQ(2U, m0.rows);

  constexpr auto v = 1.444412315;
  m0(0U, 0U)       = v;
  EXPECT_NEAR(v, m0(0U, 0U), 0.0000001);

  const auto cloned = m0.clone();
  EXPECT_NEAR(cloned(0U, 0U), m0(0U, 0U), 0.0000001);
}

TEST(MatTest, BilinearInterpolate) {
  painty::Mat<painty::vec3> m0 = painty::Mat<painty::vec3>(2U, 2U);
  m0(0, 0)                     = {1.5, 2.3, 4.5};
  m0(0, 1)                     = {1.5, 2.3, 4.5};
  m0(1, 0)                     = {2.5, 2.3, 4.5};
  m0(1, 1)                     = {2.5, 2.3, 4.5};

  const auto expected = (0.5 * m0(1, 0) + 0.5 * m0(0, 0));

  const painty::vec2 pos = {0.5, 0.5};

  EXPECT_NEAR(expected[0], painty::Interpolate(m0, pos)[0], 0.0000001);
  EXPECT_NEAR(expected[1], painty::Interpolate(m0, pos)[1], 0.0000001);
  EXPECT_NEAR(expected[2], painty::Interpolate(m0, pos)[2], 0.0000001);
}

TEST(MatTest, Resize) {
  constexpr auto testColor = 0.5;
  painty::Mat<double> m0(256U, 256U);
  for (auto& p : m0) {
    p = testColor;
  }

  const auto smaller = painty::ScaledMat(m0, 135, 94);
  for (const auto& p : smaller) {
    EXPECT_NEAR(testColor, p, 0.0001);
  }

  const auto bigger = painty::ScaledMat(m0, 2 * m0.rows, 3 * m0.cols);
  for (const auto& p : bigger) {
    EXPECT_NEAR(testColor, p, 0.0001);
  }
}
