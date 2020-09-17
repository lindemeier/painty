/**
 * @file Math.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */
#pragma once

#include <cmath>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "painty/core/Vec.hxx"

namespace painty {

template <class Float>
static constexpr auto Pi = static_cast<Float>(
  3.141592653589793238462643383279502884197169399375105820974L);

/**
 * @brief Fuzzy comparison of floating points.
 *
 * @tparam Float the floating point type.
 * @param firstValue first value for comparison.
 * @param secondValue second value for comparison.
 * @param epsilon
 * @return if the given values are fuzzy equal.
 */
template <typename Float>
typename std::enable_if<std::is_floating_point<Float>::value, bool>::type
fuzzyCompare(Float firstValue, Float secondValue, Float epsilon) {
  return std::fabs(firstValue - secondValue) < epsilon;
}

/**
 * @brief Fuzzy comparison of floating points.
 *
 * @tparam Float the floating point type.
 * @param firstValue first value for comparison.
 * @param secondValue second value for comparison.
 * @param epsilon
 *
 * @return if the given values are fuzzy equal.
 */
template <typename Float, size_t N>
typename std::enable_if<std::is_floating_point<Float>::value, bool>::type
fuzzyCompare(const std::array<Float, N>& firstValue,
             const std::array<Float, N>& secondValue, const Float epsilon) {
  auto equal = true;
  for (size_t i = 0U; ((i < N) && equal); i++) {
    equal = fuzzyCompare<Float>(firstValue[i], secondValue[i], epsilon);
  }
  return equal;
}

/**
 * @brief https://www.in.tu-clausthal.de/fileadmin/homes/techreports/ifi0505hormann.pdf Generalized barycentric
 * coordinates for arbitrary polygons
 *
 * @tparam Value
 * @param polygon list of 2d points in clock or anticlock wise order
 * @param position the 2d position to interpolate a values
 * @param values the list of values along the polygon
 *
 * @return Value the interpolated value at v
 */
template <class Value>
Value generalizedBarycentricCoordinatesInterpolate(
  const std::vector<vec2>& polygon, const vec2& position,
  const std::vector<Value>& values) {
  const auto n = polygon.size();

  constexpr auto Eps = std::numeric_limits<double>::epsilon() * 100.0;

  if (polygon.empty() || values.empty()) {
    throw std::invalid_argument("Polygon is empty");
  }
  if (polygon.size() != values.size()) {
    throw std::invalid_argument("Polygon size differs from values size");
  } else if (n == 1U) {
    return values.front();
  }

  std::vector<vec2> s(n);
  for (size_t i = 0U; i < n; ++i) {
    s[i] = polygon[i] - position;
  }
  std::vector<double> r(n);
  std::vector<double> A(n);
  std::vector<double> D(n);
  for (size_t i = 0U; i < n; ++i) {
    vec2 si1 = (i == n - 1) ? s[0] : s[i + 1];
    vec2 si  = s[i];

    r[i] = si.norm();

    if (fuzzyCompare(r[i], 0.0, Eps)) {
      return values[i];
    }

    /*
    |si.x si1.x|
    |si.y si1.y|
    */
    const auto det = si[0] * si1[1] - si1[0] * si[1];
    A[i]           = det / 2.0;
    D[i]           = si.dot(si1);

    if (fuzzyCompare(A[i], 0.0, Eps) && D[i] < 0.0) {
      double& ri1 = (i == n - 1) ? r[0] : r[i + 1];
      Value fi1   = (i == n - 1) ? values.front() : values[i + 1];
      ri1         = si1.norm();
      return (ri1 * values[i] + r[i] * fi1) * (1.0 / (r[i] + ri1));
    }
  }

  Value f  = {};
  double W = 0.;

  for (size_t i = 0U; i < n; ++i) {
    double w   = 0.;
    double A_1 = (i == 0) ? A.back() : A[i - 1];

    if (A_1 != 0.) {
      double r_1 = (i == 0) ? r.back() : r[i - 1];
      double D_1 = (i == 0) ? D.back() : D[i - 1];

      w = w + (r_1 - D_1 / r[i]) / A_1;
    }
    if (A[i] != 0.) {
      double ri1 = (i == n - 1) ? r[0] : r[i + 1];

      w = w + (ri1 - D[i] / r[i]) / A[i];
    }
    f = f + w * values[i];
    W = W + w;
  }
  if (!fuzzyCompare(W, 0.0, Eps)) {
    return f * (1.0 / W);
  }
  return values.front();
}

/**
 * @brief Cotangent hyperbolicus
 *
 * @tparam Float
 * @tparam 0
 * @param x
 * @return Float
 */
template <typename Float, typename std::enable_if_t<
                            std::is_floating_point<Float>::value, int> = 0>
Float coth(const Float& x) {
  if (x > static_cast<Float>(20.0)) {
    return static_cast<Float>(1.0);
  }
  if (std::fabs(x) > static_cast<Float>(0.0)) {
    const auto res = std::cosh(x) / std::sinh(x);
    return std::isnan(res) ? static_cast<Float>(1.0) : res;
  } else {
    return std::numeric_limits<Float>::infinity();
  }
}

template <
  typename Float, size_t N,
  typename std::enable_if_t<std::is_floating_point<Float>::value, int> = 0>
vec<Float, N> coth(const vec<Float, N>& x) {
  vec<Float, N> res;
  for (auto i = 0U; i < N; i++) {
    res[i] = coth(x[i]);
  }
}

template <typename Float, typename std::enable_if_t<
                            std::is_floating_point<Float>::value, int> = 0>
Float acoth(const Float& x) {
  if (fuzzyCompare(
        x, 1.0,
        static_cast<Float>(100.0) * std::numeric_limits<Float>::epsilon())) {
    return std::numeric_limits<Float>::infinity();
  }
  return std::log((x + static_cast<Float>(1.0)) /
                  (x - static_cast<Float>(1.0))) /
         static_cast<Float>(2.0);
}

template <
  typename Float, size_t N,
  typename std::enable_if_t<std::is_floating_point<Float>::value, int> = 0>
vec<Float, N> acoth(const vec<Float, N>& x) {
  vec<Float, N> res;
  for (auto i = 0U; i < N; i++) {
    res[i] = acoth(x[i]);
  }
}

/**
 *
 *@brief
 *
 * @tparam Float
 * @tparam 0
 * @param polygon
 * @param vertex
 * @return true
 * @return false
 *
 * Adapted from :
 * Copyright (c) 1970-2003, Wm. Randolph Franklin

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the following conditions:

    Redistributions of source code must retain the above copyright notice, this list of conditions and the following
disclaimers. Redistributions in binary form must reproduce the above copyright notice in the documentation and/or other
materials provided with the distribution. The name of W. Randolph Franklin may not be used to endorse or promote
products derived from this Software without specific prior written permission.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
template <typename Float, typename std::enable_if_t<
                            std::is_floating_point<Float>::value, int> = 0>
bool PointInPolyon(const std::vector<vec<Float, 2UL>>& polygon,
                   const vec<Float, 2UL>& vertex) {
  auto isInside = false;
  // set to last and first element
  auto p_prev = polygon.back();
  auto p_curr = polygon.front();
  // after using first and last, build line segments to end of polygon
  for (auto p_prev_it = polygon.cbegin(), p_curr_it = (polygon.cbegin() + 1UL);
       p_curr_it != polygon.cend(); p_prev_it++, p_curr_it++) {
    if (((p_curr[1U] > vertex[1U]) != (p_prev[1U] > vertex[1U])) &&
        (vertex[0U] < (p_prev[0U] - p_curr[0U]) * (vertex[1U] - p_curr[1U]) /
                          (p_prev[1U] - p_curr[1U]) +
                        p_curr[0U])) {
      isInside = !isInside;
    }
    p_prev = *p_prev_it;
    p_curr = *p_curr_it;
  }
  return isInside;
}

template <typename T>
inline T clamp(const T& x, const T& minV, const T& maxV) {
  return std::min(maxV, std::max(minV, x));
}

template <typename T>
inline T map(const T& x, const T& minX, const T& maxX, const T& minR,
             const T& maxR) {
  return (((x - minX) * (maxR - minR)) / (maxX - minX)) + minR;
}

template <typename T>
inline T lerp(const T& p0, const T& p1, T c) {
  return (T(1) - c) * p0 + c * p1;
}

template <class T>
inline T sgn(T val) {
  return (static_cast<T>(0) < val) - (val < static_cast<T>(0));
}

template <class T>
inline T smoothstep(const T edge0, const T edge1, const T x) {
  T t = clamp<T>((x - edge0) / (edge1 - edge0), 0.0, 1.0);
  return t * t * (3.0 - 2.0 * t);
}

// Gauss functions
template <class T>
inline T Gauss(const T& x, const T& sigma) {
  T sigmasquare = sigma * sigma;
  return exp(-x * x / (2 * sigmasquare)) /
         (std::sqrt(Pi<T> * static_cast<T>(2.0) * sigmasquare));
}

template <class T>
inline T Gauss(const T& x, const T& y, const T& sigma) {
  T sigmasquare = sigma * sigma;
  return std::exp(-(x * x + y * y) / (2. * sigmasquare)) /
         (Pi<T> * static_cast<T>(2.0) * sigmasquare);
}

template <class T>
inline T Gauss(const T& x_, const T& y_, const T& sigma_x, const T& sigma_y,
               const T theta, bool normalized = true) {
  const T x = x_ * cos(theta) + y_ * sin(theta);
  const T y = -x_ * sin(theta) + y_ * cos(theta);

  const T exponential = std::exp(-((x * x) / (T(2) * sigma_x * sigma_x)) +
                                 ((y * y) / (T(2) * sigma_y * sigma_y)));
  if (normalized) {
    const T norm = (Pi<T> * static_cast<T>(2.0) * sigma_x * sigma_y);
    return exponential / norm;
  }
  return exponential;
}

template <class T>
inline T Gauss1deriv(const T& dx, const T& dy, const T& sigma, const T& theta,
                     bool normalized = true) {
  const T x     = dx * cos(theta) + dy * sin(theta);
  const T y     = -dx * sin(theta) + dy * cos(theta);
  T sigmasquare = sigma * sigma;

  if (!normalized) {
    return (-(x / sigmasquare)) *
           std::exp(-(x * x + y * y) / (2. * sigmasquare));
  }

  return (-x / (Pi<T> * static_cast<T>(2.0) * sigmasquare * sigmasquare)) *
         std::exp(-(x * x + y * y) / (2. * sigmasquare));
}

template <class T>
inline T Gauss2deriv(const T& dx, const T& dy, const T& sigma, const T& theta,
                     bool normalized = true) {
  const T x     = dx * cos(theta) + dy * sin(theta);
  const T y     = -dx * sin(theta) + dy * cos(theta);
  T sigmasquare = sigma * sigma;
  T xsquare     = x * x;
  T ysquare     = y * y;

  if (!normalized) {
    return ((x - sigmasquare) / (sigmasquare * sigmasquare)) *
           std::exp(-(x * x + y * y) / (T(2) * sigmasquare));
  }

  return (-1. + (xsquare / sigmasquare)) *
         (exp(-(xsquare + ysquare) / (2. * sigmasquare)) /
          (Pi<T> * static_cast<T>(2.0) * sigmasquare * sigmasquare));
}

}  // namespace painty
