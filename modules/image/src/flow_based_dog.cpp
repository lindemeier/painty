/**
 * @file flow_based_dog.cpp
 * @author Thomas Lindemeier
 * @date 2020-08-27
 *
 *
 */
#include "painty/image/flow_based_dog.h"

#include <painty/image/edge_tangent_flow.h>

namespace painty {

namespace detail {
static void run_oabf(uint32_t pass, const Mat3d& sourceLab, Mat3d& target,
                     const Mat2d& tfm, const double sigma_d,
                     const double sigma_r) {
  const auto w = sourceLab.cols;
  const auto h = sourceLab.rows;

  for (auto y = 0; y < h; y++) {
    for (auto x = 0; x < w; x++) {
      const vec2 uv(x, y);
      const auto tangent = Interpolate(tfm, uv);
      auto t = (pass == 0) ? vec2(tangent[1U], -tangent[0U]) : tangent;

      if (std::abs(t[0U]) >= std::abs(t[1U])) {
        t[1U] = t[1U] / t[0U];
        t[0U] = 1.0;
        t[2U] = 0.;
      } else {
        t[0U] = t[0U] / t[1U];
        t[1U] = 1.0;
        t[2U] = 0;
      }

      const auto center = Interpolate(sourceLab, uv);

      auto sum = center;

      double norm      = 1.0;
      double halfWidth = (2.0 * sigma_d) / sqrt(t[0U] * t[0U] + t[1U] * t[1U]);

      for (auto d = 1; d <= halfWidth; d++) {
        const double uxn = uv[0U] + d * t[0U];
        const double uyn = uv[1U] + d * t[1U];
        const double uxp = uv[0U] - d * t[0U];
        const double uyp = uv[1U] - d * t[1U];

        const auto c0 = Interpolate(sourceLab, {uxn, uyn});
        const auto c1 = Interpolate(sourceLab, {uxp, uyp});

        const auto e0 = std::sqrt(std::pow(c0[0U] - center[0U], 2.0) +
                                  std::pow(c0[1U] - center[1U], 2.0) +
                                  std::pow(c0[2U] - center[2U], 2.0));
        const auto e1 = std::sqrt(std::pow(c1[0U] - center[0U], 2.0) +
                                  std::pow(c1[1U] - center[1U], 2.0) +
                                  std::pow(c1[2U] - center[2U], 2.0));

        const auto kerneld  = exp(-(d * d) / (2.0 * sigma_d * sigma_d));
        const auto kernele0 = exp(-(e0 * e0) / (2.0 * sigma_r * sigma_r));
        const auto kernele1 = exp(-(e1 * e1) / (2.0 * sigma_r * sigma_r));

        norm += kerneld * kernele0;
        norm += kerneld * kernele1;

        sum[0U] += kerneld * kernele0 * c0[0U];
        sum[1U] += kerneld * kernele0 * c0[1U];
        sum[2U] += kerneld * kernele0 * c0[2U];

        sum[0U] += kerneld * kernele1 * c1[0U];
        sum[1U] += kerneld * kernele1 * c1[1U];
        sum[2U] += kerneld * kernele1 * c1[2U];
      }
      sum[0U] /= norm;
      sum[1U] /= norm;
      sum[2U] /= norm;

      target(y, x) = sum;
    }
  }
}
}  // namespace detail

Mat3d FlowBasedDoG::execute(const Mat3d& imageInLab,
                            const Mat2d& edgeTangentFlow) const {
  filterBilateralOrientationAligned(imageInLab, edgeTangentFlow, _oabfSigma_d,
                                    _oabfSigma_r, _oabfIterations);
  return quantizeColors(imageInLab, _phi_q, _nbins);
}

Mat3d FlowBasedDoG::filterBilateralOrientationAligned(
  const Mat3d& imageInLab, const Mat2d& edgeTangentFlow, const double sigma_d,
  const double sigma_r, const uint32_t n) {
  Mat3d t0(imageInLab.size());
  auto t1 = imageInLab.clone();

  for (auto i = 0U; i < n; i++) {
    detail::run_oabf(0U, t1, t0, edgeTangentFlow, sigma_d, sigma_r);
    detail::run_oabf(1U, t0, t1, edgeTangentFlow, sigma_d, sigma_r);
  }

  return t1;
}

Mat3d FlowBasedDoG::quantizeColors(const Mat3d& imageInLab, const double phi_q,
                                   const uint32_t nbins) {
  const auto w = imageInLab.cols;
  const auto h = imageInLab.rows;
  Mat3d out(h, w);
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      const auto c = imageInLab(y, x);

      const auto qn = std::floor(c[0U] * static_cast<double>(nbins) + 0.5) /
                      static_cast<double>(nbins);
      const auto qs =
        painty::smoothstep(-2.0, 2.0, phi_q * (c[0U] - qn) * 100.0) - 0.5;
      const auto qc = qn + qs / static_cast<double>(nbins);
      out(y, x)     = {qc, c[1U], c[2U]};
    }
  }
  return out;
}

}  // namespace painty
