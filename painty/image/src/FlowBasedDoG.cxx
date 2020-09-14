/**
 * @file FlowBasedDoG.cxx
 * @author Thomas Lindemeier
 * @date 2020-08-27
 *
 *
 */
#include "painty/image/FlowBasedDoG.hxx"

#include "painty/image/EdgeTangentFlow.hxx"

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
      } else {
        t[0U] = t[0U] / t[1U];
        t[1U] = 1.0;
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

static void fdogAlongGradient(const Mat1d& img, Mat1d& dst, const Mat2d& tfm,
                              const double sigma_e, const double sigma_r,
                              const double tau) {
  const auto twoSigmaESquared = 2.0 * sigma_e * sigma_e;
  const auto twoSigmaRSquared = 2.0 * sigma_r * sigma_r;

  const auto w = img.cols;
  const auto h = img.rows;

  for (auto y = 0; y < h; y++) {
    for (auto x = 0; x < w; x++) {
      const vec2 uv = {x, y};
      const auto t  = tfm(clamp(static_cast<int32_t>(round(uv[1U])), 0, h - 1),
                         clamp(static_cast<int32_t>(round(uv[0U])), 0,
                               w - 1));  // nearest neighbor
      vec2 n        = {t[1U], -t[0U]};    // along gradient not tangent
      if (std::fabs(n[0U]) >= std::fabs(n[1U])) {
        n[1U] = n[1U] / n[0U];
        n[0U] = 1.0;
      } else {
        n[0U] = n[0U] / n[1U];
        n[1U] = 1.0;
      }
      const auto ht = Interpolate(img, uv);

      auto sumG0  = ht;
      auto sumG1  = ht;
      auto normG0 = 1.0;
      auto normG1 = 1.0;

      auto halfWidth = 2.0 * sigma_r / sqrt(n[0U] * n[0U] + n[1U] * n[1U]);
      for (auto d = 1; d <= halfWidth; d++) {
        // kernel for both gaussians
        vec2 kernel = {exp(-d * d / twoSigmaESquared),
                       exp(-d * d / twoSigmaRSquared)};
        normG0 += 2.0 * kernel[0U];
        normG1 += 2.0 * kernel[1U];

        const auto backwardsValue =
          Interpolate(img, {uv[0U] - d * n[0U], uv[1U] - d * n[1U]});
        const auto forwardsValue =
          Interpolate(img, {uv[0U] + d * n[0U], uv[1U] + d * n[1U]});

        // only Luminance used
        const auto accumValues = backwardsValue + forwardsValue;

        sumG0 += kernel[0] * accumValues;
        sumG1 += kernel[1] * accumValues;
      }
      sumG0 /= normG0;
      sumG1 /= normG1;

      // DoG operation
      dst(y, x) = sumG0 - tau * sumG1;
    }
  }
}

}  // namespace detail

void FlowBasedDoG::smoothAlongFlow(const Mat1d& img, Mat1d& dst,
                                   const Mat2d& tfm, const double sigma_m) {
  struct lic_t {
    vec2 p    = {0.0, 0.0};
    vec2 t    = {0.0, 0.0};
    double w  = 0.0;
    double dw = 0.0;
  };
  const auto sign = [](double x) {
    return (x <= 0.0) ? -1.0 : 1.0;
  };

  const auto step = [sign, &tfm](lic_t& s) {
    auto t = Interpolate(tfm, {s.p[0U], s.p[1U]});
    if (t.dot(s.t) < 0.0) {
      t *= -1.0;
    }
    s.t[0U] = t[0U];
    s.t[1U] = t[1U];

    s.dw = (std::fabs(t[0U]) >= std::fabs(t[1U]))
             ? std::fabs(((s.p[0U] - std::floor(s.p[0U])) - 0.5 - sign(t[0U])) /
                         t[0U])
             : std::fabs(((s.p[1U] - std::floor(s.p[1U])) - 0.5 - sign(t[1U])) /
                         t[1U]);

    s.p[0U] += t[0U] * s.dw;
    s.p[1U] += t[1U] * s.dw;
    s.w += s.dw;
  };

  const double twoSigmaMSquared = 2.0 * sigma_m * sigma_m;
  const double halfWidth        = 2.0 * sigma_m;
  const auto w                  = img.cols;
  const auto h                  = img.rows;

  for (auto y = 0; y < h; y++) {
    for (auto x = 0; x < w; x++) {
      const vec2 uv = {x, y};
      double wg     = 1.0;
      double H      = img(y, x);

      lic_t a;
      lic_t b;
      a.p[0U] = b.p[0U] = uv[0U];
      a.p[1U] = b.p[1U] = uv[1U];
      a.t               = tfm(y, x);
      b.t               = -tfm(y, x);
      a.w = b.w = 0.0;

      while (a.w < halfWidth) {
        step(a);
        double k = a.dw * exp(-a.w * a.w / twoSigmaMSquared);
        H += k * Interpolate(img, {a.p[0U], a.p[1U]});
        wg += k;
      }
      while (b.w < halfWidth) {
        step(b);
        double k = b.dw * exp(-b.w * b.w / twoSigmaMSquared);
        H += k * Interpolate(img, {b.p[0U], b.p[1U]});
        wg += k;
      }
      H /= wg;

      dst(y, x) = H;
    }
  }
}

Mat3d FlowBasedDoG::execute(const Mat3d& rgbLinear) const {
  const auto Lab =
    convertColor(rgbLinear, ColorConverter<double>::Conversion::rgb_2_CIELab);
  const auto etf = ComputeEdgeTangentFlow(
    tensor::ComputeTensors(Lab, Mat1d(), 0.0, _tensorOuterSigma));

  const auto oabf = filterBilateralOrientationAligned(
    Lab, etf, _oabfSigma_d, _oabfSigma_r, _oabfIterations);

  std::vector<Mat1d> channels;
  cv::split(oabf, channels);
  const auto dogResponse = filterFlowBasedDoG(
    channels.front(), etf, _xdogParamSigma, _xdogParamKappa * _xdogParamSigma,
    _xdogParamTau, _xdogParamSmoothingSigma);

  const auto xdog = thresholdingXDoG(dogResponse, _xdogParamEps, _xdogParamPhi);

  return overlay(
    xdog, convertColor(quantizeColors(oabf, _phi_q, _nbins),
                       ColorConverter<double>::Conversion::CIELab_2_rgb));
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
  for (auto y = 0; y < h; y++) {
    for (auto x = 0; x < w; x++) {
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

Mat1d FlowBasedDoG::thresholdingXDoG(const Mat1d& response,
                                     const double xdogParamEps,
                                     const double xdogParamPhi) {
  Mat1d out(response.size());
  for (int32_t i = 0; i < (response.cols * response.rows); i++) {
    const auto e = response(i);
    out(i) = (e > xdogParamEps) ? 1.0 : (1.0 + std::tanh(xdogParamPhi * e));
  }

  return out;
}

Mat1d FlowBasedDoG::filterFlowBasedDoG(const Mat1d& img, const Mat2d& tfm,
                                       const double sigma_e,
                                       const double sigma_r, const double tau,
                                       const double sigmaSmoothing) {
  Mat1d out(img.rows, img.cols);

  // compute DoG
  detail::fdogAlongGradient(img, out, tfm, sigma_e, sigma_r, tau);

  // smooth along etf
  Mat1d outSmooth(img.rows, img.cols);
  smoothAlongFlow(out, outSmooth, tfm, sigmaSmoothing);

  return outSmooth;
}

Mat3d FlowBasedDoG::overlay(const Mat1d& edges, const Mat3d& image) {
  const auto w = edges.cols;
  const auto h = edges.rows;

  Mat3d t0(h, w);
  for (auto y = 0; y < h; y++) {
    for (auto x = 0; x < w; x++) {
      const auto c = image(y, x);
      double e     = edges(y, x);
      t0(y, x)     = {e * c[0U], e * c[1U], e * c[2U]};
    }
  }

  return t0;
}

}  // namespace painty
