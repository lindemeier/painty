/**
 * @file Mat.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */
#pragma once

#include <memory>
#include <vector>

#include "opencv2/imgproc.hpp"
#include "painty/core/Color.h"
#include "painty/core/Vec.h"

namespace cv {  // opencv access data traits
template <typename T, int32_t R, int32_t C>
class DataType<Eigen::Matrix<T, R, C>> {
 public:
  typedef Eigen::Matrix<T, R, C> value_type;
  typedef Eigen::Matrix<T, R, C> work_type;
  typedef T channel_type;
  typedef value_type vec_type;
  enum {
    generic_type = 0,
    depth        = DataDepth<channel_type>::value,
    channels     = R * C,
    fmt          = ((channels - 1) << 8) + DataDepth<channel_type>::fmt,
    type         = CV_MAKETYPE(depth, channels)
  };
};

}  // namespace cv

namespace painty {

template <class T>
using Mat = cv::Mat_<T>;

// predefines of often used mat
// dynamic matrix
using Mat1u  = Mat<uint8_t>;
using Mat1i  = Mat<int32_t>;
using Mat1ui = Mat<uint32_t>;
using Mat1f  = Mat<float>;
using Mat1d  = Mat<double>;
using Mat2u  = Mat<vec2u>;
using Mat2i  = Mat<vec2i>;
using Mat2f  = Mat<vec2f>;
using Mat2d  = Mat<vec2>;
using Mat3u  = Mat<vec3u>;
using Mat3i  = Mat<vec3i>;
using Mat3f  = Mat<vec3f>;
using Mat3d  = Mat<vec3>;
using Mat4f  = Mat<vec4f>;
using Mat4d  = Mat<vec4>;

/**
 * @brief Access data bilinearly interpolated.
 *
 * @param position
 *
 * @return T bilinearly interpolated value at position.
 */
template <class T>
T Interpolate(const Mat<T>& input, const vec2& position,
              int borderType = cv::BORDER_REFLECT) {
  const auto x = static_cast<int32_t>(std::floor(position[0]));
  const auto y = static_cast<int32_t>(std::floor(position[1]));

  const auto x0 = cv::borderInterpolate(x, input.cols, borderType);
  const auto x1 = cv::borderInterpolate(x + 1, input.cols, borderType);
  const auto y0 = cv::borderInterpolate(y, input.rows, borderType);
  const auto y1 = cv::borderInterpolate(y + 1, input.rows, borderType);

  const auto a = position[0] - static_cast<double>(x);
  const auto c = position[1] - static_cast<double>(y);
  if constexpr (DataType<T>::dim == 1U) {
    return static_cast<T>((static_cast<double>(input(y0, x0)) * (1.0 - a) +
                           static_cast<double>(input(y0, x1)) * a) *
                            (1.0 - c) +
                          (static_cast<double>(input(y1, x0)) * (1.0 - a) +
                           static_cast<double>(input(y1, x1)) * a) *
                            c);
  } else {
    T r;
    for (auto i = 0U; i < DataType<T>::rows; i++) {
      for (auto j = 0U; j < DataType<T>::cols; j++) {
        r(i, j) = static_cast<typename DataType<T>::channel_type>(
          (static_cast<double>(input(y0, x0)(i, j)) * (1.0 - a) +
           static_cast<double>(input(y0, x1)(i, j)) * a) *
            (1.0 - c) +
          (static_cast<double>(input(y1, x0)(i, j)) * (1.0 - a) +
           static_cast<double>(input(y1, x1)(i, j)) * a) *
            c);
      }
    }
    return r;
  }
}

/**
  * @brief Return a padded copy of the mat.
  *
  * @param left
  * @param right
  * @param up
  * @param down
  * @param paddingValue
  * @return Mat<T>
  */
template <class T>
Mat<T> PaddedMat(const Mat<T>& input, const int32_t left, const int32_t right,
                 const int32_t up, const int32_t down, const T& paddingValue) {
  Mat<T> s(input.rows + up + down, input.cols + left + right);
  // initialize all to default value
  for (auto& v : s) {
    v = paddingValue;
  }
  for (auto i = 0; i < input.rows; i++) {
    for (auto j = 0; j < input.cols; j++) {
      s(i + up, j + left) = input(i, j);
    }
  }
  return s;
}

/**
 * @brief Resizes an image.
 *
 * @tparam T
 * @param input
 * @param rows
 * @param cols
 * @return Mat<T>
 */
template <class T>
Mat<T> ScaledMat(const Mat<T>& input, const int32_t rows, const int32_t cols,
                 cv::InterpolationFlags flag = cv::INTER_LANCZOS4) {
  Mat<T> resized;
  cv::resize(input, resized,
             cv::Size(static_cast<int32_t>(cols), static_cast<int32_t>(rows)),
             0.0, 0.0, flag);
  return resized;
}

template <class It1, class It2, class BinaryOperation>
void transform(It1 it1_begin, It1 it1_end, It2 it2_begin,
               BinaryOperation operation) {
  while (it1_begin != it1_end) {
    operation(*it1_begin++, *it2_begin++);
  }
}

template <class T>
Mat<vec<T, 3>> convertColor(const Mat<vec<T, 3>> input,
                            typename ColorConverter<T>::Conversion conversion) {
  ColorConverter<T> converter;

  Mat<vec<T, 3>> out(input.size());
  painty::transform(
    input.begin(), input.end(), out.begin(),
    [&converter, conversion](const painty::vec3& a, painty::vec3& b) {
      converter.convert(a, b, conversion);
    });
  return out;
}

}  // namespace painty
