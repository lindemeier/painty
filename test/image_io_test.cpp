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

TEST(ImageIoTest, ReadWriteRGB)
{
  painty::Mat<painty::vec3> linear_rgb(256, 1024);

  constexpr auto steps = 20U;
  for (uint32_t i = 0; i < linear_rgb.getRows(); i++)
  {
    for (uint32_t j = 0; j < linear_rgb.getCols(); j++)
    {
      const double p = j / static_cast<double>(linear_rgb.getCols() - 1U);

      linear_rgb(i, j) = { p, p, p };
    }
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

TEST(ImageIoTest, ReadWriteSingle)
{
  painty::Mat<double> luminance(256, 1024);

  constexpr auto steps = 20U;
  for (uint32_t i = 0; i < luminance.getRows(); i++)
  {
    for (uint32_t j = 0; j < luminance.getCols(); j++)
    {
      luminance(i, j) = j / static_cast<double>(luminance.getCols() - 1U);
    }
  }
  painty::io::imSave("luminance.png", luminance);

  painty::Mat<double> luminance_read;
  painty::io::imRead("luminance.png", luminance_read);

  EXPECT_EQ(luminance_read.getCols(), luminance.getCols());
  EXPECT_EQ(luminance_read.getRows(), luminance.getRows());

  constexpr double eps = 0.001;
  for (uint32_t i = 0; i < luminance.getRows(); i++)
  {
    for (uint32_t j = 0; j < luminance.getCols(); j++)
    {
      EXPECT_NEAR(luminance(i, j), luminance_read(i, j), eps);
    }
  }
}
