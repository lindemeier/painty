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

  for (uint32_t i = 0; i < linear_rgb.getRows(); i++) {
    for (uint32_t j = 0; j < linear_rgb.getCols(); j++) {
      const double p = j / static_cast<double>(linear_rgb.getCols() - 1U);

      linear_rgb(i, j) = {p, p, p};
    }
  }

  const auto testReadSaveRead = [&](const painty::io::ChannelDepth depth) {
    std::stringstream ss;
    ss << "linear_rgb"
       << ((depth == painty::io::ChannelDepth::BITS_16) ? "_16" : "_8")
       << ".png";
    const auto name = ss.str();
    painty::io::imSave(name, linear_rgb, depth);

    painty::Mat<painty::vec3> linear_rgb_read;
    painty::io::imRead(name, linear_rgb_read);
    painty::io::imSave("hugo.png", linear_rgb_read, depth);

    EXPECT_EQ(linear_rgb_read.getCols(), linear_rgb.getCols());
    EXPECT_EQ(linear_rgb_read.getRows(), linear_rgb.getRows());

    const double eps =
      ((depth == painty::io::ChannelDepth::BITS_16) ? (1. / 0xFFFF)
                                                    : (1. / 0xFF));
    for (uint32_t i = 0; i < linear_rgb.getRows(); i++) {
      for (uint32_t j = 0; j < linear_rgb.getCols(); j++) {
        EXPECT_NEAR(linear_rgb(i, j)[0], linear_rgb_read(i, j)[0], eps);
        EXPECT_NEAR(linear_rgb(i, j)[1], linear_rgb_read(i, j)[1], eps);
        EXPECT_NEAR(linear_rgb(i, j)[2], linear_rgb_read(i, j)[2], eps);
      }
    }
  };

  testReadSaveRead(painty::io::ChannelDepth::BITS_16);
  // testReadSaveRead(painty::io::ChannelDepth::BITS_8);
}

TEST(ImageIoTest, ReadWriteSingle) {
  painty::Mat<double> luminance(256, 1024);

  for (uint32_t i = 0; i < luminance.getRows(); i++) {
    for (uint32_t j = 0; j < luminance.getCols(); j++) {
      luminance(i, j) = j / static_cast<double>(luminance.getCols() - 1U);
    }
  }
  const auto testReadSaveRead = [&](const painty::io::ChannelDepth depth) {
    std::stringstream ss;
    ss << "luminance"
       << ((depth == painty::io::ChannelDepth::BITS_16) ? "_16" : "_8")
       << ".png";
    const auto name = ss.str();
    painty::io::imSave(name, luminance, depth);

    painty::Mat<double> luminance_read;
    painty::io::imRead(name, luminance_read);

    EXPECT_EQ(luminance_read.getCols(), luminance.getCols());
    EXPECT_EQ(luminance_read.getRows(), luminance.getRows());

    const double eps =
      ((depth == painty::io::ChannelDepth::BITS_16) ? (1. / 0xFFFF)
                                                    : (1. / 0xFF));

    for (uint32_t i = 0; i < luminance.getRows(); i++) {
      for (uint32_t j = 0; j < luminance.getCols(); j++) {
        EXPECT_NEAR(luminance(i, j), luminance_read(i, j), eps);
      }
    }
  };

  testReadSaveRead(painty::io::ChannelDepth::BITS_16);
  // testReadSaveRead(painty::io::ChannelDepth::BITS_8);
}
