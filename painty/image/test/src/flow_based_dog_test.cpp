/**
 * @file flow_based_dog_test.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-08-26
 *
 */
#include "painty/image/flow_based_dog.h"

#include "gtest/gtest.h"
#include "painty/image/edge_tangent_flow.h"
#include "painty/io/image_io.h"

TEST(FDoGTest, FDoGTestFunc) {
  painty::Mat3d image;
  // load an image as linear rgb
  painty::io::imRead("./data/test_images/field.jpg", image, true);

  painty::FlowBasedDoG fdog;
  const auto fdogRes = fdog.execute(image);

  painty::io::imSave("./data/test_images/field_oabf.jpg", fdogRes, true);
}
