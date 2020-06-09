/**
 * @file mat_test.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */
#include <gtest/gtest.h>

#include <painty/painty.h>

TEST(MatTest, Construct)
{
  painty::Mat<double> m0;

  EXPECT_TRUE(m0.isEmpty());

  EXPECT_EQ(0U, m0.getCols());
  EXPECT_EQ(0U, m0.getRows());

  m0 = painty::Mat<double>(2U, 4U);
  EXPECT_FALSE(m0.isEmpty());
  EXPECT_EQ(4U, m0.getCols());
  EXPECT_EQ(2U, m0.getRows());
}

TEST(MatTest, Assign)
{
  painty::Mat<double> m0 = painty::Mat<double>(2U, 4U);

  EXPECT_FALSE(m0.isEmpty());
  EXPECT_EQ(4U, m0.getCols());
  EXPECT_EQ(2U, m0.getRows());

  constexpr auto v = 1.444412315;
  m0(0U, 0U) = v;
  EXPECT_NEAR(v, m0(0U, 0U), 0.0000001);

  const auto cloned = m0.clone();
  EXPECT_NEAR(cloned(0U, 0U), m0(0U, 0U), 0.0000001);
}

TEST(MatTest, BilinearInterpolate)
{
  painty::Mat<painty::vec3> m0 = painty::Mat<painty::vec3>(2U, 2U);
  m0(0, 0) = { 1.5, 2.3, 4.5 };
  m0(0, 1) = { 1.5, 2.3, 4.5 };
  m0(1, 0) = { 2.5, 2.3, 4.5 };
  m0(1, 1) = { 2.5, 2.3, 4.5 };

  const auto expected = (0.5 * m0(1, 0) + 0.5 * m0(0, 0));

  const painty::vec2 pos = { 0.5, 0.5 };

  EXPECT_NEAR(expected[0], m0(pos)[0], 0.0000001);
  EXPECT_NEAR(expected[1], m0(pos)[1], 0.0000001);
  EXPECT_NEAR(expected[2], m0(pos)[2], 0.0000001);
}

TEST(MatTest, Resize)
{
  constexpr auto testColor = 0.5;
  painty::Mat<double> m0(256U, 256U);
  for (auto& p : m0.getData())
  {
    p = testColor;
  }

  const auto smaller = m0.scaled(135, 94);
  for (const auto& p : smaller.getData())
  {
    EXPECT_EQ(testColor, p);
  }

  const auto bigger = m0.scaled(2 * m0.getRows(), 3 * m0.getCols());
  for (const auto& p : bigger.getData())
  {
    EXPECT_EQ(testColor, p);
  }
}
