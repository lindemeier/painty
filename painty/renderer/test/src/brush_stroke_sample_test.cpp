/**
 * @file vec_test.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */

#include "painty/renderer/brush_stroke_sample.h"

#include "gtest/gtest.h"

TEST(BrushStrokeSample, Construct) {
  const auto sample = painty::BrushStrokeSample("./data/sample_0");

  EXPECT_EQ(800U, sample.getThicknessMap().cols);
  EXPECT_EQ(171U, sample.getThicknessMap().rows);

  EXPECT_NEAR(0.58, sample.getSampleAt({341, 101}), 0.01);
}
