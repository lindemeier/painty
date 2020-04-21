#include <gtest/gtest.h>

#include <painty/painty.h>

TEST(VecTest, Construct)
{
  painty::vec<double, 2> v = { 0.0, 0.0 };
  ASSERT_EQ(0.0, v[0]);

  v = { 0.0, 2.0 };
  ASSERT_EQ(2.0, v[1]);

  painty::vec<float, 2> v2 = { 0.0F, 0.0F };
  ASSERT_EQ(0.0, v2[0]);

  v2 = { 0.0, 2.0 };
  ASSERT_EQ(2.0, v2[1]);
}

TEST(VecTest, Norm)
{
  painty::vec<double, 2> v = { 4.0, 1.0 };

  EXPECT_NEAR(17.0, painty::normSq(v), 0.0001);
  EXPECT_NEAR(std::sqrt(17.0), painty::norm(v), 0.0001);
}

TEST(VecTest, Operators)
{
  painty::vec<double, 2> v = { 4.0, 1.0 };
  const double a = 3.0;

  painty::vec<double, 2> res = a * v;
  EXPECT_NEAR(12.0, res[0], 0.0001);
  EXPECT_NEAR(3.0, res[1], 0.0001);

  painty::vec<double, 2> v2 = { 2.0, 1.0 };
  EXPECT_NEAR(2.0, (v - v2)[0], 0.0001);
  EXPECT_NEAR(2.0, (v + v2)[1], 0.0001);

  EXPECT_NEAR(1.5, (v - 2.5)[0], 0.0001);
  EXPECT_NEAR(-1.5, (v - 2.5)[1], 0.0001);
  EXPECT_NEAR(5.5, (v + 1.5)[0], 0.0001);
  EXPECT_NEAR(2.5, (v + 1.5)[1], 0.0001);

  std::cout << v2;
}
