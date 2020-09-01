/**
 * @file spline.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-04
 *
 */
#pragma once

#include "painty/core/math.h"

namespace painty {
template <class T>
inline T LinearDerivative(const T& p0, const T& p1) {
  return p1 - p0;
}

template <class T>
inline T Linear(const T& p0, const T& p1,
                const typename DataType<T>::channel_type t) {
  using Float = typename DataType<T>::channel_type;
  return p0 * (static_cast<Float>(1.) - t) + p1 * t;
}

template <class T>
inline T CatmullRom(const T& p_1, const T& p0, const T& p1, const T& p2,
                    typename DataType<T>::channel_type t) {
  using Float = typename DataType<T>::channel_type;

  Float t2 = t * t;
  Float t3 = t2 * t;

  Float b1 = static_cast<Float>(0.5) * (-t3 + static_cast<Float>(2) * t2 - t);
  Float b2 = static_cast<Float>(0.5) *
             (static_cast<Float>(3) * t3 - static_cast<Float>(5) * t2 +
              static_cast<Float>(2));
  Float b3 = static_cast<Float>(0.5) *
             (static_cast<Float>(-3) * t3 + static_cast<Float>(4) * t2 + t);
  Float b4 = static_cast<Float>(0.5) * (t3 - t2);

  return (p_1 * b1 + p0 * b2 + p1 * b3 + p2 * b4);
}

template <class T>
inline T Cubic(const T& p_1, const T& p0, const T& p1, const T& p2,
               typename DataType<T>::channel_type t) {
  using Float = typename DataType<T>::channel_type;

  Float t2;
  T a0, a1, a2, a3;

  t2 = t * t;
  a0 = p2 - p1 - p_1 + p0;
  a1 = p_1 - p0 - a0;
  a2 = p1 - p_1;
  a3 = p0;

  return a0 * t * t2 + a1 * t2 + a2 * t + a3;
}

template <class T>
inline T CubicDerivativeFirst(const T& p_1, const T& p0, const T& p1,
                              const T& p2,
                              typename DataType<T>::channel_type t) {
  using Float = typename DataType<T>::channel_type;

  Float t2 = t * t;
  T a0     = p2 - p1 - p_1 + p0;
  T a1     = p_1 - p0 - a0;
  T a2     = p1 - p_1;

  return static_cast<Float>(3.) * a0 * t2 + static_cast<Float>(2.) * a1 * t +
         a2;
}

template <class T>
inline T CubicDerivativeSecond(const T& p_1, const T& p0, const T& p1,
                               const T& p2,
                               typename DataType<T>::channel_type t) {
  using Float = typename DataType<T>::channel_type;

  T a0 = p2 - p1 - p_1 + p0;
  T a1 = p_1 - p0 - a0;

  return static_cast<Float>(2.) * (static_cast<Float>(3.) * a0 * t + a1);
}

/**
 * @brief Spline curve evaluation for lists of data control points. More than 4 control points are allowed.
 *
 * @tparam ContainerIt
 */
template <class ContainerIt>
class SplineEval {
  using value_type = typename ContainerIt::value_type;
  using value      = typename DataType<value_type>::channel_type;

  ContainerIt _begin;
  ContainerIt _end;

 public:
  SplineEval(ContainerIt begin, ContainerIt end) : _begin(begin), _end(end) {}

  value_type linear(value u) {
    int32_t index;
    value t;
    getControl(u, index, t);
    return Linear(getIndexClamped(index), getIndexClamped(index + 1), t);
  }

  value_type linearDerivative(value u) {
    int32_t index;
    value t;
    getControl(u, index, t);
    return LinearDerivative(getIndexClamped(index), getIndexClamped(index + 1));
  }

  value_type cubic(value u) {
    int32_t index;
    value t;
    getControl(u, index, t);
    return Cubic(getIndexClamped(index - 1), getIndexClamped(index),
                 getIndexClamped(index + 1), getIndexClamped(index + 2), t);
  }

  value_type cubicDerivative(value u, int32_t order) {
    int32_t index;
    value t;
    getControl(u, index, t);
    if (order == 2) {
      return CubicDerivativeSecond(
        getIndexClamped(index - 1), getIndexClamped(index),
        getIndexClamped(index + 1), getIndexClamped(index + 2), t);
    } else {
      return CubicDerivativeFirst(
        getIndexClamped(index - 1), getIndexClamped(index),
        getIndexClamped(index + 1), getIndexClamped(index + 2), t);
    }
  }

  value cubicCurvature(value u) {
    int32_t index;
    value t;
    getControl(u, index, t);
    return CubicCurvature(getIndexClamped(index - 1), getIndexClamped(index),
                          getIndexClamped(index + 1),
                          getIndexClamped(index + 2), t);
  }

  value_type catmullRom(value u) {
    int32_t index;
    value t;
    getControl(u, index, t);
    return CatmullRom(getIndexClamped(index - 1), getIndexClamped(index),
                      getIndexClamped(index + 1), getIndexClamped(index + 2),
                      t);
  }

 private:
  void getControl(const value u, int32_t& index, value& t) const {
    auto n = std::distance(_begin, _end);

    const value x = (n - 1) * u;
    index         = static_cast<int32_t>(x);
    t             = x - std::floor(x);
  }

  const value_type& getIndexClamped(int32_t index) const {
    auto n = std::distance(_begin, _end);

    if (index < 0)
      return *_begin;
    else if (index >= static_cast<int32_t>(n))
      return *(_end - 1);
    else
      return *(_begin + index);
  }
};
}  // namespace painty
