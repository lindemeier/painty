/**
 * @file vec.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */
#ifndef PAINTY_VEC_H
#define PAINTY_VEC_H

#include <array>
#include <cmath>
#include <ostream>
#include <type_traits>

#include <painty/types.h>

namespace painty
{
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
using vec = std::array<T, N>;

using vec1 = vec<double, 1U>;
using vec2 = vec<double, 2U>;
using vec3 = vec<double, 3U>;
using vec4 = vec<double, 4U>;

template <typename T, size_t N>
class DataType<vec<T, N> >
{
public:
  using channel_type = T;
  static constexpr int32_t rows = N;
  static constexpr int32_t cols = 1;
  static constexpr int32_t dim = rows * cols;
};

/**
 * @brief Compute dot product of given vectors.
 *
 * @param a
 * @param b
 * @return T
 */
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
T dot(const vec<T, N>& a, const vec<T, N>& b)
{
  T d = static_cast<T>(0.0);

  for (size_t i = 0U; i < N; i++)
  {
    d = d + a[i] * b[i];
  }
  return d;
}

/**
 * @brief Compute dot product of given vectors.
 *
 * @param a
 * @param b
 * @return T
 */
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
T operator*(const vec<T, N>& a, const vec<T, N>& b)
{
  return dot(a, b);
}

/**
 * @brief Multiply the vector by a scalar component wise.
 *
 * @param a
 * @param b
 * @return T
 */
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
vec<T, N> operator*(const T a, const vec<T, N>& b)
{
  vec<T, N> p = b;
  for (auto& v : p)
  {
    v *= a;
  }
  return p;
}

/**
 * @brief Multiply the vector by a scalar component wise.
 *
 * @param a
 * @param b
 * @return T
 */
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
vec<T, N> operator*(const vec<T, N>& b, const T a)
{
  return a * b;
}

/**
 * @brief Add vectors component wise.
 *
 * @param a
 * @param b
 * @return T
 */
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
vec<T, N> operator+(const vec<T, N>& a, const vec<T, N>& b)
{
  vec<T, N> d = a;

  for (size_t i = 0U; i < N; i++)
  {
    d[i] += b[i];
  }
  return d;
}

/**
 * @brief Add a scalar to a vector component wise.
 *
 * @param a
 * @param b
 * @return T
 */
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
vec<T, N> operator+(const vec<T, N>& a, const T b)
{
  vec<T, N> d = a;

  for (size_t i = 0U; i < N; i++)
  {
    d[i] += b;
  }
  return d;
}

/**
 * @brief Subtract a scalar from a vector component wise.
 *
 * @param a
 * @param b
 * @return T
 */
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
vec<T, N> operator-(const vec<T, N>& a, const vec<T, N>& b)
{
  vec<T, N> d = a;

  for (size_t i = 0U; i < N; i++)
  {
    d[i] -= b[i];
  }
  return d;
}

/**
 * @brief Subtract a scalar from a vector component wise.
 *
 * @param a
 * @param b
 * @return T
 */
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
vec<T, N> operator-(const vec<T, N>& a, const T b)
{
  vec<T, N> d = a;

  for (size_t i = 0U; i < N; i++)
  {
    d[i] -= b;
  }
  return d;
}

/**
 * @brief Computes the l2 norm squared.
 *
 * @param v
 * @return T
 */
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
T normSq(const vec<T, N>& v)
{
  return v * v;
}

/**
 * @brief Computes the l2 norm.
 *
 * @param v
 * @return T
 */
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
T norm(const vec<T, N>& v)
{
  return std::sqrt(v * v);
}

/**
 * @brief Normalize vector.
 *
 * @param v
 * @return T the normalized vector.
 */
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
vec<T, N> normalized(const vec<T, N>& v)
{
  const auto nSq = normSq(v);
  if (std::fabs(nSq) < (std::numeric_limits<T>::epsilon() * static_cast<T>(100.0)))
  {
    return v;
  }
  vec<T, N> c = v;
  const auto scale = static_cast<T>(1.0) / std::sqrt(nSq);
  for (auto& a : c)
  {
    a *= scale;
  }

  return c;
}

template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
std::ostream& operator<<(std::ostream& os, const vec<T, N> a)
{
  os << "(";
  for (size_t i = 0U; i < N - 1U; i++)
  {
    os << a[i] << ", ";
  }
  os << a.back() << ")";
  return os;
}

}  // namespace painty

using painty::operator*;
using painty::operator+;
using painty::operator-;

using painty::operator<<;

#endif  // PAINTY_VEC_H
