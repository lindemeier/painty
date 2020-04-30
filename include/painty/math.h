/**
 * @file math.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */
#ifndef PAINTY_MATH_H
#define PAINTY_MATH_H

// #include <vector>

#include <painty/vec.h>

namespace painty
{
// https://www.in.tu-clausthal.de/fileadmin/homes/techreports/ifi0505hormann.pdf
// Generalized barycentric coordinates for arbitrary polygons
template <class Value>
Value generalized_barycentric_coordinates_interpolate(const std::vector<vec2>& polygon, const vec2& v,
                                                      const std::vector<Value>& values)
{
  const auto n = polygon.size();

  if (polygon.empty() || values.empty())
  {
    return {};
  }
  else if (polygon.size() != values.size())
  {
    return {};
  }
  else if (n == 1U)
  {
    return values.front();
  }

  auto valuesBegin = values.cbegin();

  std::vector<vec2> s(n);
  for (size_t i = 0U; i < n; ++i)
  {
    s[i] = polygon[i] - v;
  }
  std::vector<double> r(n);
  std::vector<double> A(n);
  std::vector<double> D(n);
  for (size_t i = 0U; i < n; ++i)
  {
    vec2 si1 = (i == n - 1) ? s[0] : s[i + 1];
    vec2 si = s[i];

    r[i] = norm(si);

    if (r[i] == 0.)
    {
      return *(valuesBegin + i);
    }

    /*
    |si.x si1.x|
    |si.y si1.y|
    */
    const auto det = si[0] * si1[1] - si1[0] * si[1];
    A[i] = det / 2.0;
    D[i] = dot(si, si1);

    if (A[i] == 0.0 && D[i] < 0.0)
    {
      double& ri1 = (i == n - 1) ? r[0] : r[i + 1];
      Value fi1 = (i == n - 1) ? values.front() : values[i + 1];
      ri1 = norm(si1);
      return (ri1 * values[i] + r[i] * fi1) / (r[i] + ri1);
    }
  }

  Value f = {};
  double W = 0.;

  for (size_t i = 0U; i < n; ++i)
  {
    double w = 0.;
    double A_1 = (i == 0) ? A.back() : A[i - 1];

    if (A_1 != 0.)
    {
      double r_1 = (i == 0) ? r.back() : r[i - 1];
      double D_1 = (i == 0) ? D.back() : D[i - 1];

      w = w + (r_1 - D_1 / r[i]) / A_1;
    }
    if (A[i] != 0.)
    {
      double ri1 = (i == n - 1) ? r[0] : r[i + 1];

      w = w + (ri1 - D[i] / r[i]) / A[i];
    }
    f = f + w * values[i];
    W = W + w;
  }
  return f / W;
}
}  // namespace painty

#endif  // PAINTY_MATH_H
