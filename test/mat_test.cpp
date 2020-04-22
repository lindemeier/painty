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
