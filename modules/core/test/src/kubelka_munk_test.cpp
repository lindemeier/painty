/**
 * @file kubelka_munk_test.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-14
 *
 */

#include <gtest/gtest.h>
#include <painty/core/kubelka_munk.h>

TEST(KubelkaMunk, Reflectance) {
  constexpr auto Eps = 0.00001;

  const auto d                     = 0.5;
  const painty::vec<double, 3U> k  = {0.2, 0.1, 0.22};
  const painty::vec<double, 3U> s  = {0.124, 0.658, 0.123};
  const painty::vec<double, 3U> r0 = {0.65, 0.2, 0.2146};
  const auto r1                    = painty::ComputeReflectance(k, s, r0, d);
  EXPECT_NEAR(r1[0], 0.541596, Eps);
  EXPECT_NEAR(r1[1], 0.343822, Eps);
  EXPECT_NEAR(r1[2], 0.206651, Eps);

  EXPECT_NEAR(painty::ComputeReflectance(k, s, r0, 0.0)[0], r0[0], Eps);

  const painty::vec<double, 3U> s2 = {0.0, 0.0, 0.0};
  EXPECT_NEAR(painty::ComputeReflectance(k, s2, r0, d)[0], 0.53217499727638173,
              Eps);
}
