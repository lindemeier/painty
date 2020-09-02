/**
 * @file BrushStrokeSampleTest.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */

#include "gtest/gtest.h"
#include "painty/renderer/BrushStrokeSample.h"

TEST(BrushStrokeSample, Construct) {
  const auto sample = painty::BrushStrokeSample("./data/sample_0");

  EXPECT_EQ(800U, sample.getThicknessMap().cols);
  EXPECT_EQ(171U, sample.getThicknessMap().rows);

  EXPECT_NEAR(0.2900139589503905, sample.getSampleAt({341, 101}), 0.01);
}
