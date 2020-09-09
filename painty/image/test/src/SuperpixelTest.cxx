/**
 * @file SuperpixelTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-08-26
 *
 */
#include "gtest/gtest.h"
#include "painty/core/Color.hxx"
#include "painty/image/Superpixel.hxx"
#include "painty/io/ImageIO.hxx"

TEST(SuperPixelTest, SuperPixelExecuteTest) {
  painty::Mat3d image;
  // load an image as linear rgb
  painty::io::imRead("./data/test_images/field.jpg", image, true);
  // convert to Lab color space
  painty::Mat3d labImage(image.size());
  painty::ColorConverter<double> converter;
  painty::transform(image.begin(), image.end(), labImage.begin(),
                    [&converter](const painty::vec3& rgb, painty::vec3& lab) {
                      converter.rgb2lab(rgb, lab);
                    });

  painty::SuperpixelSegmentation segmentation;
  segmentation.setExtractionStrategy(
    painty::SuperpixelSegmentation::ExtractionStrategy::SLICO_POISSON_WEIGHTED);

  painty::Mat3d gImage(labImage.size());
  for (auto& a : gImage) {
    a = {0.0, 0.0, 0.0};
  }
  segmentation.extract(labImage, gImage,
                       painty::Mat<uint8_t>(labImage.size(), 255U), 30.0);

  segmentation.getSegmentationOutlined(image);

  // TODO load a segmented image and compare it to
}
