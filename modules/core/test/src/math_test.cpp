/**
 * @file math_test.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */

#include <gtest/gtest.h>
#include "painty/core/math.h"

TEST(MathTest, GeneralizedBarycentricCoordinatesInterpolate) {
  {
    std::vector<painty::vec2> polygon = {
      {-0.5, -0.5}, {0.5, -0.5}, {0.5, 0.5}, {-0.5, 0.5}};
    std::vector<double> values  = {0.0, 0.0, 1.0, 1.0};
    const painty::vec2 position = {0.0, 0.0};
    const auto interpolated_color_at_position =
      painty::generalizedBarycentricCoordinatesInterpolate(polygon, position,
                                                           values);
    EXPECT_NEAR(interpolated_color_at_position, 0.5, 0.000001);
  }

  {
    std::vector<painty::vec2> polygon = {
      {-0.5, -0.5}, {0.5, -0.5}, {0.5, 0.5}, {-0.5, 0.5}};
    std::vector<double> values  = {0.0, 0.0, 1.0};
    const painty::vec2 position = {0.0, 0.0};
    try {
      painty::generalizedBarycentricCoordinatesInterpolate(polygon, position,
                                                           values);
      FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument& err) {
      EXPECT_EQ(err.what(),
                std::string("Polygon size differs from values size"));
    } catch (...) {
      FAIL() << "Expected std::invalid_argument";
    }
  }

  {
    std::vector<painty::vec2> polygon = {{-0.5, -0.5}};
    std::vector<double> values        = {0.563};
    const painty::vec2 position       = {0.0, 0.0};
    const auto interpolated_color_at_position =
      painty::generalizedBarycentricCoordinatesInterpolate(polygon, position,
                                                           values);
    EXPECT_NEAR(interpolated_color_at_position, 0.563, 0.000001);
  }

  {
    std::vector<painty::vec2> polygon = {};
    std::vector<double> values        = {};
    const painty::vec2 position       = {0.0, 0.0};
    try {
      painty::generalizedBarycentricCoordinatesInterpolate(polygon, position,
                                                           values);
      FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument& err) {
      EXPECT_EQ(err.what(), std::string("Polygon is empty"));
    } catch (...) {
      FAIL() << "Expected std::invalid_argument";
    }
  }
}

TEST(MathTest, fuzzyCompare) {
  EXPECT_TRUE(painty::fuzzyCompare(0.0, 0.0, 0.0001));
  EXPECT_TRUE(painty::fuzzyCompare(0.0, 0.0001, 0.0002));
  EXPECT_FALSE(painty::fuzzyCompare(0.0, 0.0002, 0.0001));
  EXPECT_FALSE(painty::fuzzyCompare(192391223.123123123,
                                    192391223.123123123 - 0.001, 0.0001));
  EXPECT_TRUE(painty::fuzzyCompare(192391223.123123123,
                                   192391223.123123123 - 0.0001, 0.001));

  std::array<double, 2UL> a = {0.0, 0.0};
  std::array<double, 2UL> b = {0.1, 0.8};
  EXPECT_TRUE(painty::fuzzyCompare(a, b, 1.0));
}

TEST(MathTest, coth) {
  EXPECT_EQ(painty::coth(0.0), std::numeric_limits<double>::infinity());

  constexpr auto Eps = 0.00001;
  EXPECT_NEAR(painty::coth(0.176), 5.7403640542091674, Eps);
  EXPECT_NEAR(painty::coth(0.09742), 10.297285488897547, Eps);
  EXPECT_NEAR(painty::coth(1263.148), 1.0, Eps);
  EXPECT_NEAR(painty::coth(18.0), 1.0, Eps);
}

TEST(MathTest, PointInPoly) {
  std::vector<painty::vec<double, 2UL>> polygon = {
    {-1.0, -1.0}, {1.0, -1.0}, {1.0, 1.0}, {-1.0, 1.0}};

  EXPECT_TRUE(painty::PointInPolyon(polygon, {0.5, 0.5}));
  EXPECT_TRUE(painty::PointInPolyon(polygon, {0.0, 0.0}));
  EXPECT_TRUE(painty::PointInPolyon(polygon, {-1.0, -1.0}));
  EXPECT_FALSE(painty::PointInPolyon(polygon, {-1.1, -1.0}));
  EXPECT_FALSE(painty::PointInPolyon(polygon, {5.1, 12.0}));
  EXPECT_FALSE(painty::PointInPolyon(polygon, {0.2, -12.412}));
}
