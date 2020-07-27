/**
 * @file vec.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */
#pragma once

#include <painty/core/types.h>

#include <array>
#include <cmath>
#include <eigen3/Eigen/Dense>
#include <ostream>
#include <type_traits>

namespace painty {
/**
 * @brief
 *
 * @tparam Type
 * @tparam R rows: Eigen has ints as template params
 */
template <typename Type, int32_t R>
using vec = Eigen::Matrix<Type, R, static_cast<int32_t>(1)>;

using vec1 = vec<double, static_cast<int32_t>(1)>;
using vec2 = vec<double, static_cast<int32_t>(2)>;
using vec3 = vec<double, static_cast<int32_t>(3)>;
using vec4 = vec<double, static_cast<int32_t>(4)>;

template <typename T, int32_t N>
class DataType<vec<T, N> > {
 public:
  using channel_type            = T;
  static constexpr int32_t rows = N;
  static constexpr int32_t cols = 1;
  static constexpr int32_t dim  = rows * cols;
};

}  // namespace painty
