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

#include <painty/painty.h>

TEST(MathTest, GeneralizedBarycentricCoordinatesInterpolate)
{
  {
    std::vector<painty::vec2> polygon = { { -0.5, -0.5 }, { 0.5, -0.5 }, { 0.5, 0.5 }, { -0.5, 0.5 } };
    std::vector<double> values = { 0.0, 0.0, 1.0, 1.0 };
    const painty::vec2 position = { 0.0, 0.0 };
    const auto interpolated_color_at_position =
        painty::generalized_barycentric_coordinates_interpolate(polygon, position, values);
    EXPECT_NEAR(interpolated_color_at_position, 0.5, 0.000001);
  }

  {
    std::vector<painty::vec2> polygon = { { -0.5, -0.5 }, { 0.5, -0.5 }, { 0.5, 0.5 }, { -0.5, 0.5 } };
    std::vector<double> values = { 0.0, 0.0, 1.0 };
    const painty::vec2 position = { 0.0, 0.0 };
    const auto interpolated_color_at_position =
        painty::generalized_barycentric_coordinates_interpolate(polygon, position, values);
    EXPECT_NEAR(interpolated_color_at_position, 0.0, 0.000001);
  }

  {
    std::vector<painty::vec2> polygon = { { -0.5, -0.5 } };
    std::vector<double> values = { 0.563 };
    const painty::vec2 position = { 0.0, 0.0 };
    const auto interpolated_color_at_position =
        painty::generalized_barycentric_coordinates_interpolate(polygon, position, values);
    EXPECT_NEAR(interpolated_color_at_position, 0.563, 0.000001);
  }

  {
    std::vector<painty::vec2> polygon = {};
    std::vector<double> values = {};
    const painty::vec2 position = { 0.0, 0.0 };
    const auto interpolated_color_at_position =
        painty::generalized_barycentric_coordinates_interpolate(polygon, position, values);
    EXPECT_NEAR(interpolated_color_at_position, 0.0, 0.000001);
  }
}

TEST(MathTest, FuzzyEqual)
{
  EXPECT_TRUE(painty::fuzzyEqual(0.0, 0.0, 0.0001));
  EXPECT_TRUE(painty::fuzzyEqual(0.0, 0.0001, 0.0002));
  EXPECT_FALSE(painty::fuzzyEqual(0.0, 0.0002, 0.0001));
  EXPECT_FALSE(painty::fuzzyEqual(192391223.123123123, 192391223.123123123 - 0.001, 0.0001));
  EXPECT_TRUE(painty::fuzzyEqual(192391223.123123123, 192391223.123123123 - 0.0001, 0.001));

  std::array<double, 2UL> a = { 0.0, 0.0 };
  std::array<double, 2UL> b = { 0.1, 0.8 };
  EXPECT_TRUE(painty::fuzzyEqual(a, b, 1.0));
}
