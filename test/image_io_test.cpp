/**
 * @file image_io_test.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */

#include <gtest/gtest.h>

#include <painty/painty.h>

TEST(ImageIoTest, ReadWrite)
{
  painty::Mat<painty::vec3> linear_rgb(1024, 768);

  constexpr painty::vec3 color = { 0.5, 0.001, 0.999 };

  for (auto& pixel : linear_rgb.getData())
  {
    pixel = color;
  }
  painty::io::imSave("linear_rgb.png", linear_rgb);

  painty::Mat<painty::vec3> linear_rgb_read;
  painty::io::imRead("linear_rgb.png", linear_rgb_read);

  EXPECT_EQ(linear_rgb_read.getCols(), linear_rgb.getCols());
  EXPECT_EQ(linear_rgb_read.getRows(), linear_rgb.getRows());

  constexpr double eps = 0.001;
  for (uint32_t i = 0; i < linear_rgb.getRows(); i++)
  {
    for (uint32_t j = 0; j < linear_rgb.getCols(); j++)
    {
      EXPECT_NEAR(linear_rgb(i, j)[0], linear_rgb_read(i, j)[0], eps);
      EXPECT_NEAR(linear_rgb(i, j)[1], linear_rgb_read(i, j)[1], eps);
      EXPECT_NEAR(linear_rgb(i, j)[2], linear_rgb_read(i, j)[2], eps);
    }
  }
}
