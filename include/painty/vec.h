#ifndef PAINTY_VEC_H
#define PAINTY_VEC_H

#include <cmath>
#include <ostream>

namespace painty
{
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
using vec = std::array<T, N>;

using vec1 = vec<double, 1U>;
using vec2 = vec<double, 2U>;
using vec3 = vec<double, 3U>;
using vec4 = vec<double, 4U>;

/**
 * @brief Compute dot product of given vectors.
 *
 * @param a
 * @param b
 * @return T
 */
template <class T, size_t N, template <class, size_t> class Vec>
T dot(const Vec<T, N>& a, const Vec<T, N>& b)
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
template <class T, size_t N, template <class, size_t> class Vec>
T operator*(const Vec<T, N>& a, const Vec<T, N>& b)
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
template <class T, size_t N, template <class, size_t> class Vec>
Vec<T, N> operator*(const T a, const Vec<T, N>& b)
{
  Vec<T, N> p = b;
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
template <class T, size_t N, template <class, size_t> class Vec>
Vec<T, N> operator*(const Vec<T, N>& b, const T a)
{
  return a * b;
}

template <class T, size_t N, template <class, size_t> class Vec>
Vec<T, N> operator+(const Vec<T, N>& a, const Vec<T, N>& b)
{
  Vec<T, N> d = a;

  for (size_t i = 0U; i < N; i++)
  {
    d[i] += b[i];
  }
  return d;
}

template <class T, size_t N, template <class, size_t> class Vec>
Vec<T, N> operator+(const Vec<T, N>& a, const T b)
{
  Vec<T, N> d = a;

  for (size_t i = 0U; i < N; i++)
  {
    d[i] += b;
  }
  return d;
}

template <class T, size_t N, template <class, size_t> class Vec>
Vec<T, N> operator+(const T b, const Vec<T, N> a)
{
  return a + b;
}

template <class T, size_t N, template <class, size_t> class Vec>
Vec<T, N> operator-(const Vec<T, N>& a, const Vec<T, N>& b)
{
  Vec<T, N> d = a;

  for (size_t i = 0U; i < N; i++)
  {
    d[i] -= b[i];
  }
  return d;
}

template <class T, size_t N, template <class, size_t> class Vec>
Vec<T, N> operator-(const Vec<T, N>& a, const T b)
{
  Vec<T, N> d = a;

  for (size_t i = 0U; i < N; i++)
  {
    d[i] -= b;
  }
  return d;
}

template <class T, size_t N, template <class, size_t> class Vec>
Vec<T, N> operator-(const T b, const Vec<T, N> a)
{
  return a - b;
}

/**
 * @brief Computes the l2 norm squared.
 *
 * @param v
 * @return T
 */
template <class T, size_t N, template <class, size_t> class Vec>
T normSq(const Vec<T, N>& v)
{
  return v * v;
}

/**
 * @brief Computes the l2 norm.
 *
 * @param v
 * @return T
 */
template <class T, size_t N, template <class, size_t> class Vec>
T norm(const Vec<T, N>& v)
{
  return std::sqrt(v * v);
}

/**
 * @brief Normalize vector.
 *
 * @param v
 * @return T the normalized vector.
 */
template <class T, size_t N, template <class, size_t> class Vec>
T normalized(const Vec<T, N>& v)
{
  return std::sqrt(v * v);
}

template <class T, size_t N, template <class, size_t> class Vec>
std::ostream& operator<<(std::ostream& os, const Vec<T, N> a)
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
