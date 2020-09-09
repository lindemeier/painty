/**
 * @file FlowBasedDoGTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-08-26
 *
 */
#include "gtest/gtest.h"
#include "painty/image/EdgeTangentFlow.hxx"
#include "painty/image/FlowBasedDoG.hxx"
#include "painty/io/ImageIO.hxx"

TEST(FDoGTest, FDoGTestFunc) {
  painty::Mat3d image;
  // load an image as linear rgb
  painty::io::imRead("./data/test_images/field.jpg", image, true);

  painty::FlowBasedDoG fdog;
  const auto fdogRes = fdog.execute(image);

  painty::io::imSave("./data/test_images/field_oabf.jpg", fdogRes, true);
}
