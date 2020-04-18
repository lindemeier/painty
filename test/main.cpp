#include <gtest/gtest.h>

double square(const double x)
{
  return x * x;
}

TEST(SquareTest, PositiveNos)
{
  ASSERT_EQ(9.0, square(3.0));
  ASSERT_EQ(4.0, square(2.0));
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
