/**
 * @file edge_tangent_flow.cpp
 * @author Thomas Lindemeier
 * @brief
 *
 * @date 2020-07-31
 *
 *
 */
#include "painty/image/edge_tangent_flow.h"

#include <random>

#include "painty/core/math.h"
#include "painty/image/convolution.h"

namespace painty {
namespace tensor {

double GetMinEigenvalue(const vec3& tensor) {
  const double E   = tensor[0];
  const double F   = tensor[1];
  const double G   = tensor[2];
  const double det = ::std::sqrt(::std::pow(E - G, 2.) + 4. * F * F);
  return (E + G - det) * 0.5;
}

double GetMaxEigenvalue(const vec3& tensor) {
  const double E   = tensor[0];
  const double F   = tensor[1];
  const double G   = tensor[2];
  const double det = ::std::sqrt(::std::pow(E - G, 2.0) + 4.0 * F * F);
  return (E + G + det) * 0.5;
}

vec2 GetMinEigenVector(const vec3& tensor) {
  const double E = tensor[0];
  const double F = tensor[1];
  const double G = tensor[2];

  double det = ::std::sqrt(::std::pow(E - G, 2.0) + 4.0 * F * F);
  vec2 v     = {2.0 * F, G - E - det};

  return v;
}

vec2 GetMaxEigenvector(const vec3& tensor) {
  const double E = tensor[0];
  const double F = tensor[1];
  const double G = tensor[2];

  const double det = std::sqrt(std::pow(E - G, 2.0) + 4.0 * F * F);
  return vec2(2.0 * F, G - E + det);
}

/**
 *
 * @param image source image, preferably in CIELab space
 * @param mask computaion mask, zero pixels are skipped
 * @param innerSigma sigma for Gauss blurring the gradients
 * @param outerSigma sigma for gauss blurring the tensors
 * @param relaxationThreshold gradients smaller than this values get interpolated
 * @return
 */
Mat3d ComputeTensors(const Mat3d& image, const Mat1d& mask, double innerSigma,
                     double outerSigma, const double spatialSigma,
                     const double colorSigma) {
  // compute derivation according to "Image and Video Abstraction by Coherence-Enhancing Filtering"
  // http://onlinelibrary.wiley.com/doi/10.1111/j.1467-8659.2011.01882[0]/full
  Mat3d dxTemp(image.size()), dyTemp(image.size());
  cv::Sobel(image, dxTemp, dxTemp.depth(), 1, 0, 3);
  cv::Sobel(image, dyTemp, dyTemp.depth(), 0, 1, 3);

  // inner blur
  if (innerSigma > 0) {
    if (mask.data) {
      cv::Mat_<double> maskB(mask.size());
      for (int32_t i = 0; i < (dxTemp.cols * dxTemp.rows); i++) {
        maskB(i) = mask(i) > 0 ? 1. : 0.;
        if (mask(i) > 0) {
          dxTemp(i) = dxTemp(i);
          dyTemp(i) = dyTemp(i);
        } else {
          dxTemp(i) << 0., 0., 0.;
          dyTemp(i) << 0., 0., 0.;
        }
      }
      cv::GaussianBlur(maskB, maskB, cv::Size(-1, -1), innerSigma, 0.0,
                       cv::BORDER_REFLECT);
      cv::GaussianBlur(dxTemp, dxTemp, cv::Size(-1, -1), innerSigma, 0.0,
                       cv::BORDER_REFLECT);
      cv::GaussianBlur(dyTemp, dyTemp, cv::Size(-1, -1), innerSigma, 0.0,
                       cv::BORDER_REFLECT);

      for (int32_t i = 0; i < dxTemp.cols * dxTemp.rows; i++) {
        const auto allowed = (maskB(i) > 0);
        dxTemp(i)          = (allowed) ? dxTemp(i) / maskB(i) : dxTemp(i);
        dyTemp(i)          = (allowed) ? dyTemp(i) / maskB(i) : dyTemp(i);
      }
    } else {
      cv::GaussianBlur(dxTemp, dxTemp, cv::Size(-1, -1), innerSigma, 0.0,
                       cv::BORDER_REFLECT);
      cv::GaussianBlur(dyTemp, dyTemp, cv::Size(-1, -1), innerSigma, 0.0,
                       cv::BORDER_REFLECT);
    }
  }

  // second order tensors
  cv::Mat_<double> dx2(dxTemp.size()), dy2(dxTemp.size()), dxy(dxTemp.size());
  for (int32_t i = 0; i < dxTemp.cols * dxTemp.rows; i++) {
    vec3 g0 = dxTemp(i);
    vec3 g1 = dyTemp(i);

    dx2(i) = g0.dot(g0);
    dy2(i) = g1.dot(g1);
    dxy(i) = g0.dot(g1);
  }

  // outer blur
  if (outerSigma > 0) {
    const int32_t k_size = -1;
    if (mask.data) {
      Mat<double> maskB(mask.size());
      for (int32_t i = 0; i < dxTemp.cols * dxTemp.rows; i++) {
        maskB(i) = mask(i) > 0 ? 1. : 0.;
        if (mask(i) > 0) {
          dx2(i) = dx2(i);
          dy2(i) = dy2(i);
          dxy(i) = dxy(i);
        } else {
          dx2(i) = 0.0;
          dy2(i) = 0.0;
          dxy(i) = 0.0;
        }
      }
      cv::GaussianBlur(maskB, maskB, cv::Size(k_size, k_size), outerSigma, 0.0,
                       cv::BORDER_REFLECT);
      cv::GaussianBlur(dx2, dx2, cv::Size(k_size, k_size), outerSigma, 0.0,
                       cv::BORDER_REFLECT);
      cv::GaussianBlur(dy2, dy2, cv::Size(k_size, k_size), outerSigma, 0.0,
                       cv::BORDER_REFLECT);
      cv::GaussianBlur(dxy, dxy, cv::Size(k_size, k_size), outerSigma, 0.0,
                       cv::BORDER_REFLECT);

      for (int32_t i = 0; i < (dxTemp.cols * dxTemp.rows); i++) {
        const auto allowed = (maskB(i) > 0);
        dx2(i)             = (allowed) ? dx2(i) / maskB(i) : dx2(i);
        dy2(i)             = (allowed) ? dy2(i) / maskB(i) : dy2(i);
        dxy(i)             = (allowed) ? dxy(i) / maskB(i) : dxy(i);
      }
    } else {
      cv::GaussianBlur(dx2, dx2, cv::Size(k_size, k_size), outerSigma, 0.0,
                       cv::BORDER_REFLECT);
      cv::GaussianBlur(dy2, dy2, cv::Size(k_size, k_size), outerSigma, 0.0,
                       cv::BORDER_REFLECT);
      cv::GaussianBlur(dxy, dxy, cv::Size(k_size, k_size), outerSigma, 0.0,
                       cv::BORDER_REFLECT);
    }
  }
  Mat3d tensors(dx2.size());
  for (auto i = 0U; i < tensors.total(); i++) {
    const auto index = static_cast<int32_t>(i);
    tensors(index)   = {dx2(index), dxy(index), dy2(index)};
  }
  // normalize
  double mag = 0.0;
  for (auto i = 0U; i < tensors.total(); i++) {
    mag = std::max(mag, tensors(static_cast<int32_t>(i)).norm());
  }
  if (mag > 0.0) {
    double magScale = 1. / mag;
    for (auto i = 0U; i < tensors.total(); i++) {
      const auto index = static_cast<int32_t>(i);
      tensors(index)[0] *= magScale;
      tensors(index)[1] *= magScale;
      tensors(index)[2] *= magScale;
    }
  }

  if (spatialSigma > 0.0 && colorSigma > 0.0) {
    // smooth according to source image
    Mat3f fTensors;
    tensors.convertTo(fTensors, CV_32FC3, 1.0);
    Mat3f fImage;
    image.convertTo(fImage, CV_32FC3, 1.0);
    fTensors = filterDomainTransform(fImage, fTensors, spatialSigma, colorSigma,
                                     cv::ximgproc::DTF_NC);
    fTensors.convertTo(tensors, CV_64FC3, 1.0);
  }

  return tensors;
}
}  // namespace tensor

/**
 * @brief Compute an edge tangent flow field from structure tensors.
 * The tensors get reduced to the minimal eigenvector, which follows edges tangentially in images.
 *
 * @param structureTensorField
 *
 * @return Mat2d
 */
Mat2d ComputeEdgeTangentFlow(const Mat3d& structureTensorField) {  // create etf
  Mat2d etf(structureTensorField.rows, structureTensorField.cols);

  for (int32_t i = 0;
       i < (structureTensorField.rows * structureTensorField.cols); ++i) {
    const auto E = std::isnan(structureTensorField(i)[0])
                     ? 0.
                     : structureTensorField(i)[0];  // isnan check
    const auto F = std::isnan(structureTensorField(i)[1])
                     ? 0.
                     : structureTensorField(i)[1];  // isnan check
    const auto G = std::isnan(structureTensorField(i)[2])
                     ? 0.
                     : structureTensorField(i)[2];  // isnan check

    const auto det = ::std::sqrt(::std::pow(E - G, 2.) + 4. * F * F);
    // const auto lambda = (E + G - det) * 0.5;

    const vec2 v = {2. * F, G - E - det};

    const auto m = v.norm();
    if (!fuzzyCompare(m, 0.0,
                      std::numeric_limits<double>::epsilon() * 1000.0)) {
      etf(i) = v.normalized();
    } else {
      etf(i) = {0.0, 1.0};
    }
  }
  return etf;
}

/**
 * @brief Visualize vector fields (edge tangent flow) using salt and pepper noise convoluted along the vector field.
 *
 * @param etf
 * @param sigmaL
 * @return Mat1d
 */
Mat1d lineIntegralConv(const Mat2d& etf, const double sigmaL) {
  const int32_t w = etf.cols;
  const int32_t h = etf.rows;
  Mat1d out(h, w);

  const double step = 1.0;

  const auto l = static_cast<int32_t>(
    2.0 * std::floor(std::sqrt(-std::log(0.1) * 2.0 * (sigmaL * sigmaL))) +
    1.0);

  Mat1d noise(h / 4, w / 4);
  {
    std::random_device generator;
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    for (size_t i = 0; i < noise.total(); i++) {
      noise(static_cast<int32_t>(i)) =
        (distribution(generator) < 0.5) ? 0.0 : 1.0;
    }
    cv::resize(noise, noise, etf.size(), 0., 0., cv::INTER_NEAREST);
  }

  //#pragma omp parallel for
  for (int32_t y = 0; y < h; y++) {
    for (int32_t x = 0; x < w; x++) {
      double c = 0;

      vec2 v0 = etf(y, x);

      vec2 xy_ = {x, y};
      double g = 0;

      // forward
      for (int32_t i = 0; i < l / 2; i++) {
        vec2 v1 = Interpolate(etf, xy_, cv::BORDER_REFLECT);

        if (v1.dot(v0) < 0.0)
          v1 *= (-1.0);

        xy_[0] += v1[0] * step;
        xy_[1] += v1[1] * step;

        if (std::isnan(xy_[0]) || std::isnan(xy_[1]) || xy_[0] < 0.0 ||
            xy_[0] >= w || xy_[1] < 0.0 || xy_[1] >= h)
          break;

        v0 = v1;

        double gw = std::exp(-(i * i) / (2.0 * sigmaL * sigmaL));
        c += gw *
             noise(static_cast<int32_t>(xy_[1]), static_cast<int32_t>(xy_[0]));
        g += gw;
      }
      v0  = -1.0 * etf(y, x);
      xy_ = {x, y};
      // backward
      for (int32_t i = 0; i < l / 2; i++) {
        vec2 v1 = -1.0 * Interpolate(etf, xy_, cv::BORDER_REFLECT);

        if (v1.dot(v0) < 0.0)
          v1 *= (-1.0);

        xy_[0] += v1[0] * step;
        xy_[1] += v1[1] * step;

        if (std::isnan(xy_[0]) || std::isnan(xy_[1]) || xy_[0] < 0.0 ||
            xy_[0] >= w || xy_[1] < 0.0 || xy_[1] >= h)
          break;

        v0 = v1;

        double gw = std::exp(-(i * i) / (2.0 * sigmaL * sigmaL));
        c += gw *
             noise(static_cast<int32_t>(xy_[1]), static_cast<int32_t>(xy_[0]));
        g += gw;
      }
      out(y, x) = (g > 0.0) ? c / g : 0.0;
    }
  }

  return out;
}
}  // namespace painty
