/**
 * @file convolution.h
 * @author Thomas Lindemeier
 * @brief
 *
 * @date 2020-07-31
 *
 */
#pragma once

#include <painty/core/math.h>
#include <painty/image/mat.h>

#include <opencv2/ximgproc/edge_filter.hpp>

namespace painty {

template <class T>
inline int32_t gaussKernelSizeFromSigma(const T sigma) {
  return static_cast<int32_t>(
    2.0 * std::floor(std::sqrt(-std::log(0.1) * 2 * (sigma * sigma))) + 1.0);
}

template <class T>
inline T gaussSigmaFromKernelSize(const T size) {
  return static_cast<T>(0.3 * ((size - 1) * 0.5 - 1.0) + 0.8);
}

inline int32_t gaussKernelRadiusFromSigma(const double sigma) {
  return (gaussKernelSizeFromSigma(sigma) - 1) / 2;
}

template <class T>
inline T gaussSigmaFromKernelRadius(const int32_t radius) {
  return gaussSigmaFromKernelSize<T>(radius * 2 + 1);
}

Mat1d createGauss1stDerivativeKernel(const double sigma, const double theta,
                                     bool normalized = true);

Mat1d createGauss2ndDerivativeKernel(const double sigma, const double theta,
                                     bool normalized = true);
/**
 *  article{DBLP:journals/saj/GwetuTV14,
    author    = {Mandlenkosi Victor Gwetu and
                 Jules{-}Raymond Tapamo and
                 Serestina Viriri},
    title     = {Segmentation of retinal blood vessels using normalized Gabor filters
                 and automatic thresholding},
    journal   = {South African Computer Journal},
    volume    = {55},
    pages     = {12--24},
    year      = {2014},
    url       = {http://reference.sabinet.co.za/document/EJC163970},
    timestamp = {Wed, 25 Feb 2015 16:36:57 +0100},
    biburl    = {http://dblp.uni-trier.de/rec/bib/journals/saj/GwetuTV14},
    bibsource = {dblp computer science bibliography, http://dblp.org}
 }
 * @param lambda represents the wavelength of the sinusoidal factor
 * @param theta represents the orientation of the normal to the parallel stripes
 * @param gamma the ratio in direction of the filter, gamma=1 gives isotropic kernel
 * @param psi is the phase offset
 * @param bandwidth ratio sigma/lambda the smaller the bandwidth, the more stripes you will get
 * @return a normalized gabor kernel
 */
Mat<double> createGaborKernel(double lambda, double theta, double gamma,
                              double psi, double bandwidth);

/**
 * 2D convolution of images with a given kernel.
 * Wraps the OpenCV filter2D function with standard params and
 * computes the convolution instead of correlation (what filter2D originally does).
 * Since we use OpenCV's filter2D, we get the best performance since it makes use of the FFT.
 *
 * @tparam T
 * @param source source image
 * @param out result
 * @param kernel kernel used for the convolution
 */
template <class T>
Mat<T> convolve2d(const Mat<T>& source, const Mat1d& kernel) {
  Mat1d convKernel;
  cv::flip(kernel, convKernel, -1);
  Mat<T> out;
  /*
  The function applies an arbitrary linear filter to an image. In-place operation is supported.
  When the aperture is partially outside the image, the function interpolates outlier pixel values according to the specified border mode.
  The function does actually compute correlation, not the convolution:
  That is, the kernel is not mirrored around the anchor point.
  If you need a real convolution, flip the kernel using flip() and set the new anchor to (kernel.cols - anchor.x - 1, kernel.rows - anchor.y - 1) .
  */
  cv::filter2D(source, out, -1, convKernel,
               cv::Point(convKernel.cols - (convKernel.cols / 2) - 1,
                         convKernel.rows - (convKernel.rows / 2) - 1),
               0.0, cv::BORDER_REFLECT);
  return out;
}

/**
 *
 * @tparam N0 number of channels for guide image
 * @tparam N1 number of channels for source and destination image, up to 4 channels
 * @param guide guided image (also called as joint image) with floating-point 32-bit depth
 * @param source filtering image with floating-point 32-bit depth
 * @param sigmaSpatial
 * @param sigmaColor
 * @param type one form three modes DTF_NC, DTF_RF and DTF_IC which corresponds to three modes for filtering 2D signals in the article.(see below)
 * The NC filter is ideal for stylization and abstraction, since it accurately smoothes similar image regions while preserving and sharpening relevant edges.
 * Finally, for edge-aware interpolation (e.g., colorization and recoloring), the RF filter produces the best results due to its infinite impulse response, which propagates information across the whole image lattice.
 * For applications where sharpening of edges is not desirable (e.g., tone mapping and detail manipulation), the IC and RF filters produce results of equal quality as the state-of-the-art techniques
 * @return
 */
template <int32_t N0, int32_t N1>
Mat<vec<float, N1>> filterDomainTransform(
  const Mat<vec<float, N0>>& guide, const Mat<vec<float, N1>>& source,
  const double sigmaSpatial, const double sigmaColor,
  cv::ximgproc::EdgeAwareFiltersList type = cv::ximgproc::DTF_NC) {
  Mat<vec<float, N1>> dst;
  cv::ximgproc::dtFilter(guide, source, dst, sigmaSpatial, sigmaColor, type);
  return dst;
}

template <class T>
Mat<T> differencesOfGaussians(const Mat<T>& input, const double sigma,
                              const double tau = 0.99) {
  Mat<T> d0, d1;
  cv::GaussianBlur(input, d0, cv::Size(-1, -1), sigma);
  cv::GaussianBlur(input, d1, cv::Size(-1, -1), 1.6 * sigma);

  return d0 - tau * d1;
}

template <class T>
Mat<T> computeGaborEnergy(const Mat<T>& source, const double lambda,
                          const int32_t nrAngles,
                          const double bandwidth = 1.0) {
  Mat<T> superposition(source.size());

  for (int32_t i = 0; i < nrAngles; i++) {
    const double theta = map<double>(i, 0, nrAngles, 0.0, Pi<double>);

    const auto gaussEnergy0 =
      convolve2d(source, createGaborKernel(lambda, theta, 0.75, 0., bandwidth));
    const auto gaussEnergy1 = convolve2d(
      source,
      createGaborKernel(lambda, theta, 0.75, -(Pi<double> / 2.), bandwidth));

    for (int32_t l = 0; l < source.cols * source.rows; l++) {
      superposition(l) = std::max(std::sqrt(T(std::pow(gaussEnergy0(l), 2.0) +
                                              std::pow(gaussEnergy1(l), 2.0))),
                                  superposition(l));
    }
  }
  return superposition;
}
}  // namespace painty
