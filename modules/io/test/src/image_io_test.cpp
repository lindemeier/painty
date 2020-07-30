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

#include <opencv2/highgui.hpp>

TEST(ImageIoTest, ReadWriteRGB) {
  painty::Mat<painty::vec3> linear_rgb(1, 0xFF);

  for (int32_t i = 0; i < linear_rgb.rows; i++) {
    for (int32_t j = 0; j < linear_rgb.cols; j++) {
      const double p = j / static_cast<double>(linear_rgb.cols - 1);

      linear_rgb(i, j) = {p, p, p};
    }
  }

  const auto testReadSaveRead = [&](const std::string& fileType,
                                    const double eps, bool convert_sRGB) {
    std::stringstream ss;
    ss << ((convert_sRGB) ? "srgb." : "linear_rgb.") << fileType;
    const auto name = ss.str();
    painty::io::imSave(name, linear_rgb, convert_sRGB);

    painty::Mat<painty::vec3> linear_rgb_read;
    painty::io::imRead(name, linear_rgb_read, convert_sRGB);

    EXPECT_EQ(linear_rgb_read.cols, linear_rgb.cols);
    EXPECT_EQ(linear_rgb_read.rows, linear_rgb.rows);

    for (int32_t i = 0; i < linear_rgb.rows; i++) {
      for (int32_t j = 0; j < linear_rgb.cols; j++) {
        EXPECT_NEAR(linear_rgb(i, j)[0], linear_rgb_read(i, j)[0], eps);
        EXPECT_NEAR(linear_rgb(i, j)[1], linear_rgb_read(i, j)[1], eps);
        EXPECT_NEAR(linear_rgb(i, j)[2], linear_rgb_read(i, j)[2], eps);
      }
    }
  };

  testReadSaveRead("png", 1. / 0xFFFF, true);
  testReadSaveRead("png", 1. / 0xFFFF, false);
  testReadSaveRead("jpg", 1. / 0xFF, true);
  testReadSaveRead("jpg", 1. / 0xFF, false);
}

TEST(ImageIoTest, ReadWriteSingle) {
  painty::Mat<double> lum(1, 0xFF);

  for (int32_t i = 0; i < lum.rows; i++) {
    for (int32_t j = 0; j < lum.cols; j++) {
      lum(i, j) = j / static_cast<double>(lum.cols - 1);
    }
  }

  const auto testReadSaveRead = [&](const std::string& fileType,
                                    const double eps, bool convert_sRGB) {
    std::stringstream ss;
    ss << ((convert_sRGB) ? "s_lum." : "linear_lum.") << fileType;
    const auto name = ss.str();
    painty::io::imSave(name, lum, convert_sRGB);

    painty::Mat<double> lum_read;
    painty::io::imRead(name, lum_read, convert_sRGB);

    EXPECT_EQ(lum_read.cols, lum.cols);
    EXPECT_EQ(lum_read.rows, lum.rows);

    for (int32_t i = 0; i < lum.rows; i++) {
      for (int32_t j = 0; j < lum.cols; j++) {
        EXPECT_NEAR(lum(i, j), lum_read(i, j), eps);
        EXPECT_NEAR(lum(i, j), lum_read(i, j), eps);
        EXPECT_NEAR(lum(i, j), lum_read(i, j), eps);
      }
    }
  };

  testReadSaveRead("png", 1. / 0xFFFF, true);
  testReadSaveRead("png", 1. / 0xFFFF, false);
  testReadSaveRead("jpg", 1. / 0xFF, true);
  testReadSaveRead("jpg", 1. / 0xFF, false);
}
