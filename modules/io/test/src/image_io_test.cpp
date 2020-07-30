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
  painty::Mat<painty::vec3> linear_rgb(256, 1024);

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
  // testReadSaveRead("jpg", 1. / 0xFF, true);
  // testReadSaveRead("jpg", 1. / 0xFF, false);

  painty::Mat<painty::vec3> read;
  painty::io::imRead("/home/tsl/Pictures/140467.png", read, true);
  painty::io::imSave("/home/tsl/Pictures/140467_saved.png", read, true);
  // for (auto& v : read) {
  //   std::swap(v[0U], v[2U]);
  // }
  // cv::imshow("read", read);
  // cv::waitKey(0);
}

// TEST(ImageIoTest, ReadWriteSingle) {
//   painty::Mat<double> luminance(256, 1024);

//   for (int32_t i = 0; i < luminance.rows; i++) {
//     for (int32_t j = 0; j < luminance.cols; j++) {
//       luminance(i, j) = j / static_cast<double>(luminance.cols - 1);
//     }
//   }
//   const auto testReadSaveRead = [&]() {
//     std::stringstream ss;
//     ss << "luminance.png";
//     const auto name = ss.str();
//     painty::io::imSave(name, luminance);

//     painty::Mat<double> luminance_read;
//     painty::io::imRead(name, luminance_read);

//     EXPECT_EQ(luminance_read.cols, luminance.cols);
//     EXPECT_EQ(luminance_read.rows, luminance.rows);

//     const double eps = 1. / 0xFFFF;

//     for (int32_t i = 0; i < luminance.rows; i++) {
//       for (int32_t j = 0; j < luminance.cols; j++) {
//         EXPECT_NEAR(luminance(i, j), luminance_read(i, j), eps);
//       }
//     }
//   };

//   testReadSaveRead();
// }
