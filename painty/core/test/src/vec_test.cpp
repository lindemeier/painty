/**
 * @file vec_test.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */

#include "painty/core/vec.h"

#include "gtest/gtest.h"

TEST(VecTest, Construct) {
  painty::vec<double, 2> v = {0.0, 0.0};
  ASSERT_EQ(0.0, v[0]);

  v = {0.0, 2.0};
  ASSERT_EQ(2.0, v[1]);

  painty::vec<float, 2> v2 = {0.0F, 0.0F};
  ASSERT_EQ(0.0, v2[0]);

  v2 = {0.0, 2.0};
  ASSERT_EQ(2.0, v2[1]);
}
