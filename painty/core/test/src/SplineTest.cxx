/**
 * @file SplineTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */

#include "gtest/gtest.h"
#include "painty/core/Spline.hxx"

TEST(SplineTest, BaseFunctions) {
  painty::vec<double, 2> v0 = {0.5, 0.2};
  painty::vec<double, 2> v1 = {2.0, 1.0};

  const auto r       = painty::LinearDerivative(v0, v1);
  constexpr auto Eps = 0.0000001;
  EXPECT_NEAR(1.5, r[0], Eps);
  EXPECT_NEAR(0.8, r[1], Eps);

  EXPECT_NEAR((v0[0] + v1[0]) / 2.0, painty::Linear(v0, v1, 0.5)[0], Eps);
  EXPECT_NEAR((v0[1] + v1[1]) / 2.0, painty::Linear(v0, v1, 0.5)[1], Eps);

  EXPECT_NEAR(3.5552, painty::CatmullRom(0.0, 2.0, 4.5, 5.9, 0.6), Eps);

  EXPECT_NEAR(3.4903999999999997, painty::Cubic(0.0, 2.0, 4.5, 5.9, 0.6), Eps);

  EXPECT_NEAR(1.6919999999999997,
              painty::CubicDerivativeFirst(0.0, 2.0, 4.5, 5.9, 0.6), Eps);

  EXPECT_NEAR(1.4399999999999995,
              painty::CubicDerivativeSecond(0.0, 2.0, 4.5, 5.9, 0.6), Eps);

  std::vector<painty::vec<double, 2> > points = {
    {0.12, 0.176}, {0.42, 0.23}, {0.56, 0.32}, {0.89, 0.4}, {1.12, 0.41}};
  painty::SplineEval<std::vector<painty::vec<double, 2> >::const_iterator>
    spline(points.cbegin(), points.cend());
  EXPECT_NEAR(0.36543999999999999, spline.catmullRom(0.2)[0], Eps);
  EXPECT_NEAR(0.21603200000000003, spline.catmullRom(0.2)[1], Eps);
}
