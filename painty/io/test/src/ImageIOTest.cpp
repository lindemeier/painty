/**
 * @file ImageIOTest.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */

#include "gtest/gtest.h"
#include "opencv2/highgui.hpp"
#include "painty/core/Color.h"
#include "painty/io/ImageIO.h"

TEST(ImageIoTest, ReadWriteRGB) {
  painty::Mat<painty::vec3> linear_rgb(1, 0xFF);

  for (int32_t i = 0; i < linear_rgb.rows; i++) {
    for (int32_t j = 0; j < linear_rgb.cols; j++) {
      const double p = j / static_cast<double>(linear_rgb.cols - 1);

      linear_rgb(i, j) = {p, p, p};
    }
  }

  const auto testReadSaveRead = [&](const std::string& fileType,
                                    const double eps) {
    std::stringstream ss;
    ss << "/tmp/test_rgb." << fileType;
    const auto& compare_img = linear_rgb;
    const auto name         = ss.str();
    painty::io::imSave(name, linear_rgb, false);

    painty::Mat<painty::vec3> read_image;
    painty::io::imRead(name, read_image, false);

    EXPECT_EQ(read_image.cols, compare_img.cols);
    EXPECT_EQ(read_image.rows, compare_img.rows);

    for (int32_t i = 0; i < compare_img.rows; i++) {
      for (int32_t j = 0; j < compare_img.cols; j++) {
        EXPECT_NEAR(compare_img(i, j)[0], read_image(i, j)[0], eps);
        EXPECT_NEAR(compare_img(i, j)[1], read_image(i, j)[1], eps);
        EXPECT_NEAR(compare_img(i, j)[2], read_image(i, j)[2], eps);
      }
    }
  };

  testReadSaveRead("png", 1. / 0xFFFF);
  testReadSaveRead("jpg", 1. / 0xFF);
}

TEST(ImageIoTest, ReadWriteSingle) {
  painty::Mat<double> lum(1, 0xFF);

  for (int32_t i = 0; i < lum.rows; i++) {
    for (int32_t j = 0; j < lum.cols; j++) {
      lum(i, j) = j / static_cast<double>(lum.cols - 1);
    }
  }
  const auto testReadSaveRead = [&](const std::string& fileType,
                                    const double eps) {
    std::stringstream ss;
    ss << "/tmp/test_lum." << fileType;
    const auto& compare_img = lum;
    const auto name         = ss.str();
    painty::io::imSave(name, lum, false);

    painty::Mat<double> lum_read;
    painty::io::imRead(name, lum_read, false);

    EXPECT_EQ(lum_read.cols, compare_img.cols);
    EXPECT_EQ(lum_read.rows, compare_img.rows);

    for (int32_t i = 0; i < compare_img.rows; i++) {
      for (int32_t j = 0; j < compare_img.cols; j++) {
        EXPECT_NEAR(compare_img(i, j), lum_read(i, j), eps);
      }
    }
  };

  testReadSaveRead("png", 1. / 0xFFFF);
  testReadSaveRead("jpg", 1. / 0xFF);
}
