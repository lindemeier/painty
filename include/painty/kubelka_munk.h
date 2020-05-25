/**
 * @file kubelka_munk.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */
#ifndef PAINTY_KUBELKA_MUNK_H
#define PAINTY_KUBELKA_MUNK_H

#include <painty/math.h>

namespace painty
{
/**
 * @brief Compute reflectance from absorption, scattering, layer thickness and substrate reflectance R.
 *
 * @param K absorption
 * @param S_in scattering
 * @param R0 substrate reflectance
 * @param d layer thickness
 *
 * @return vec<Float, N>
 */
template <typename Float, int32_t N, typename std::enable_if_t<std::is_floating_point<Float>::value, int> = 0>
vec<Float, N> ComputeReflectance(const vec<Float, N>& K, const vec<Float, N>& S_in, const vec<Float, N>& R0,
                                 const Float d)
{
  if (std::fabs(d) < (std::numeric_limits<Float>::epsilon() * static_cast<Float>(10000.0)))
  {
    return R0;
  }

  vec<Float, N> S;
  for (auto i = 0U; i < N; i++)
  {
    S[i] = (std::fabs(S_in[i]) > std::numeric_limits<Float>::epsilon() * static_cast<Float>(10000.0)) ?
               S_in[i] :
               static_cast<Float>(0.00000000001);
  }

  vec<Float, N> K_S;
  for (auto i = 0U; i < N; i++)
  {
    K_S[i] = K[i] / S[i];
  }

  vec<Float, N> a;
  for (auto i = 0U; i < N; i++)
  {
    a[i] = static_cast<Float>(1.0) + K_S[i];
  }

  vec<Float, N> asq;
  for (auto i = 0U; i < N; i++)
  {
    asq[i] = a[i] * a[i];
  }

  vec<Float, N> b;
  for (auto i = 0U; i < N; i++)
  {
    const auto v = asq[i] - static_cast<Float>(1.0);

    b[i] = (v < static_cast<Float>(0.0)) ? static_cast<Float>(0.0) : std::sqrt(v);
  }

  vec<Float, N> bSh;
  for (auto i = 0U; i < N; i++)
  {
    bSh[i] = b[i] * S[i] * d;
  }

  vec<Float, N> bcothbSh;
  for (auto i = 0U; i < N; i++)
  {
    bcothbSh[i] = b[i] * coth(bSh[i]);
  }
  vec<Float, N> R;
  for (auto i = 0U; i < N; i++)
  {
    R[i] = (static_cast<Float>(1.0) - R0[i] * (a[i] - bcothbSh[i])) / (a[i] - R0[i] + bcothbSh[i]);
  }

  return R;
}
}  // namespace painty

#endif  // PAINTY_KUBELKA_MUNK_H
