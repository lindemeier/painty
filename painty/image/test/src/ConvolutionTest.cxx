/**
 * @file ConvolutionTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-08-26
 *
 */
#include "gtest/gtest.h"
#include "painty/image/Convolution.hxx"
#include "painty/io/ImageIO.hxx"

TEST(ConvolutionTest, Convolve2dTest) {
  painty::Mat3d image;
  // load an image as linear rgb
  painty::io::imRead("./data/test_images/field.jpg", image, true);

  const auto convolved =
    painty::convolve2d(image, painty::createGauss2ndDerivativeKernel(1.0, 0.0));

  const auto differencesOfGaussians =
    painty::differencesOfGaussians(image, 3.0);

  painty::Mat1d single(image.size());
  const auto Lab = painty::convertColor(
    image, painty::ColorConverter<double>::Conversion::rgb_2_CIELab);
  std::vector<painty::Mat1d> channels;
  cv::split(Lab, channels);
  auto computeGaborEnergy =
    painty::computeGaborEnergy(channels.front(), 3.0, 4);

  const auto oabf = painty::smoothOABF(Lab);

  // TODO compare to serialized results
}
