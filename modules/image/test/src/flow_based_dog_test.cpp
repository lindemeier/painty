/**
 * @file flow_based_dog_test.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-08-26
 *
 */
#include <gtest/gtest.h>
#include <painty/image/edge_tangent_flow.h>
#include <painty/image/flow_based_dog.h>
#include <painty/io/image_io.h>

TEST(FDoGTest, FDoGTestFunc) {
  painty::Mat3d image;
  // load an image as linear rgb
  painty::io::imRead("./data/test_images/field.jpg", image, true);

  const auto Lab = painty::convertColor(
    image, painty::ColorConverter<double>::Conversion::rgb_2_CIELab);

  const auto etf = painty::ComputeEdgeTangentFlow(
    painty::tensor::ComputeTensors(Lab, painty::Mat1d(), 0.0, 0.5, 20.0, 30.0));

  painty::FlowBasedDoG fdog;
  const auto fdogRes = fdog.execute(Lab, etf);

  painty::io::imSave(
    "./data/test_images/field_oabf.jpg",
    painty::convertColor(
      fdogRes, painty::ColorConverter<double>::Conversion::CIELab_2_rgb),
    true);
}
