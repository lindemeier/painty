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
#include <painty/io/image_io.h>

TEST(ImageIoTest, ReadWriteRGB) {
  painty::Mat<painty::vec3> linear_rgb(256, 1024);

  for (int32_t i = 0; i < linear_rgb.rows; i++) {
    for (int32_t j = 0; j < linear_rgb.cols; j++) {
      const double p = j / static_cast<double>(linear_rgb.cols - 1);

      linear_rgb(i, j) = {p, p, p};
    }
  }

  const auto testReadSaveRead = [&]() {
    std::stringstream ss;
    ss << "linear_rgb.png";
    const auto name = ss.str();
    painty::io::imSave(name, linear_rgb);

    painty::Mat<painty::vec3> linear_rgb_read;
    painty::io::imRead(name, linear_rgb_read);

    EXPECT_EQ(linear_rgb_read.cols, linear_rgb.cols);
    EXPECT_EQ(linear_rgb_read.rows, linear_rgb.rows);

    const double eps = 1. / 0xFFFF;
    for (int32_t i = 0; i < linear_rgb.rows; i++) {
      for (int32_t j = 0; j < linear_rgb.cols; j++) {
        EXPECT_NEAR(linear_rgb(i, j)[0], linear_rgb_read(i, j)[0], eps);
        EXPECT_NEAR(linear_rgb(i, j)[1], linear_rgb_read(i, j)[1], eps);
        EXPECT_NEAR(linear_rgb(i, j)[2], linear_rgb_read(i, j)[2], eps);
      }
    }
  };

  testReadSaveRead();
}

TEST(ImageIoTest, ReadWriteSingle) {
  painty::Mat<double> luminance(256, 1024);

  for (int32_t i = 0; i < luminance.rows; i++) {
    for (int32_t j = 0; j < luminance.cols; j++) {
      luminance(i, j) = j / static_cast<double>(luminance.cols - 1);
    }
  }
  const auto testReadSaveRead = [&]() {
    std::stringstream ss;
    ss << "luminance.png";
    const auto name = ss.str();
    painty::io::imSave(name, luminance);

    painty::Mat<double> luminance_read;
    painty::io::imRead(name, luminance_read);

    EXPECT_EQ(luminance_read.cols, luminance.cols);
    EXPECT_EQ(luminance_read.rows, luminance.rows);

    const double eps = 1. / 0xFFFF;

    for (int32_t i = 0; i < luminance.rows; i++) {
      for (int32_t j = 0; j < luminance.cols; j++) {
        EXPECT_NEAR(luminance(i, j), luminance_read(i, j), eps);
      }
    }
  };

  testReadSaveRead();
}
