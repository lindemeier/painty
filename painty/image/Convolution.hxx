/**
 * @file Convolution.h
 * @author Thomas Lindemeier
 * @brief
 *
 * @date 2020-07-31
 *
 */
#pragma once

#include "painty/core/Math.hxx"
#include "painty/image/Mat.hxx"

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

Mat1d createGauss1stDerivativeKernel(double sigma, double theta,
                                     bool normalized = true);

Mat1d createGauss2ndDerivativeKernel(double sigma, double theta,
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

template <class T>
Mat<T> differencesOfGaussians(const Mat<T>& input, const double sigma,
                              const double tau = 0.99) {
  Mat<T> d0;
  Mat<T> d1;
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

/**
 * @brief Smoothes an image using the orientation aligned bilateral filter.
 * (Kyprianidis, J. E., and Kang, H. Image and video abstraction by coherence-enhancing filtering. Computer Graphics Forum 30, 2 (Apr. 2011), 593–602)
 *
 * @param labSource input image in CIELab color space.
 * @param sigmaSpatial the sigma for the spatial blur
 * @param sigmaColor sigma for the color blur
 * @param sigmaFlow sigma for the flow field blur
 * @param nIterations number of iterations to run
 * @return Mat3d
 */
auto smoothOABF(const Mat3d& labSource, const Mat1d& mask = Mat1d(),
                const double sigmaSpatial = 3.0, const double sigmaColor = 4.25,
                const double sigmaFlow = 3.0, const uint32_t nIterations = 5U)
  -> Mat3d;

}  // namespace painty
