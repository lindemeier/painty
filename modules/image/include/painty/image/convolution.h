/**
 * @file convolution.h
 * @author Thomas Lindemeier
 * @brief
 *
 * @date 2020-07-31
 *
 */
#pragma once

#include <painty/image/mat.h>

#include <opencv2/ximgproc/edge_filter.hpp>

namespace painty {
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
}  // namespace painty
