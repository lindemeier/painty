/**
 * @file Convolution.cxx
 * @author Thomas Lindemeier
 * @date 2020-08-27
 *
 */
#include "painty/image/Convolution.hxx"

#include "painty/image/EdgeTangentFlow.hxx"
#include "painty/image/FlowBasedDoG.hxx"

namespace painty {

Mat1d createGauss1stDerivativeKernel(const double sigma, const double theta,
                                     bool normalized) {
  const auto size = gaussKernelSizeFromSigma(sigma * 2.5);

  cv::Mat_<double> kernel(size, size);

  int32_t halfW = (kernel.rows - 1) / 2;
  int32_t halfC = (kernel.cols - 1) / 2;

  for (int r = -halfW; r <= halfW; ++r) {
    for (int c = -halfC; c <= halfC; ++c) {
      kernel(r + halfW, c + halfC) =
        Gauss1deriv(static_cast<double>(c), static_cast<double>(r), sigma,
                    theta, normalized);
    }
  }
  return kernel;
}

Mat1d createGauss2ndDerivativeKernel(const double sigma, const double theta,
                                     bool normalized) {
  const auto size = gaussKernelSizeFromSigma(sigma * 2.5);

  cv::Mat_<double> kernel(size, size);

  int32_t halfW = (kernel.rows - 1) / 2;
  int32_t halfC = (kernel.cols - 1) / 2;

  for (int r = -halfW; r <= halfW; ++r) {
    for (int c = -halfC; c <= halfC; ++c) {
      kernel(r + halfW, c + halfC) =
        Gauss2deriv(static_cast<double>(c), static_cast<double>(r), sigma,
                    theta, normalized);
    }
  }
  return kernel;
}

Mat<double> createGaborKernel(double lambda, double theta, double gamma,
                              double psi, double bandwidth) {
  const double sigLa =
    (1.0 / Pi<double>)*std::sqrt(std::log(2.0) / 2.0) *
    ((std::pow(2., bandwidth) + 1.) / (std::pow(2.0, bandwidth) - 1.0));
  const double sigma = sigLa * lambda;  // gaussian envelope
  Mat1d kernel =
    cv::getGaborKernel(cv::Size(-1, -1), sigma, theta, lambda, gamma, psi);

  // Algorithm 2
  double sumPos = 0.0;
  double sumNeg = 0.0;
  for (uint32_t i = 0; i < kernel.total(); ++i) {
    const double kernel_i = kernel(static_cast<int32_t>(i));
    if (kernel_i >= 0.) {
      sumPos += kernel_i;
    } else {
      sumNeg += std::abs(kernel_i);
    }
  }
  for (uint32_t i = 0U; i < kernel.total(); ++i) {
    const double kernel_i = kernel(static_cast<int32_t>(i));
    if (kernel_i >= 0.0) {
      kernel(static_cast<int32_t>(i)) = kernel_i / sumPos;
    } else {
      kernel(static_cast<int32_t>(i)) = kernel_i / sumNeg;
    }
  }

  return kernel;
}

auto smoothOABF(const Mat3d& labSource, const Mat1d& mask,
                const double sigmaSpatial, const double sigmaColor,
                const double sigmaFlow, const uint32_t nIterations) -> Mat3d {
  if ((sigmaColor <= 0.0) || (sigmaSpatial <= 0.0)) {
    return labSource.clone();
  }
  const auto etf = ComputeEdgeTangentFlow(
    tensor::ComputeTensors(labSource, mask, 0.0, sigmaFlow));

  return FlowBasedDoG::filterBilateralOrientationAligned(
    labSource, etf, sigmaSpatial, sigmaColor, nIterations);
}
}  // namespace painty
