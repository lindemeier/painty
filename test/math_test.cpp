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
