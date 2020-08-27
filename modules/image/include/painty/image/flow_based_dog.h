/**
 * @file flow_based_dog.h
 * @author Thomas Lindemeier
 * @date 2020-08-27
 *
 *
 */
#pragma once
#include <painty/image/mat.h>

namespace painty {
/**
 * @brief
 *
 */
class FlowBasedDoG {
 public:
  Mat3d execute(const Mat3d& imageInLab, const Mat2d& edgeTangentFlow) const;

  static Mat3d filterBilateralOrientationAligned(const Mat3d& imageInLab,
                                                 const Mat2d& edgeTangentFlow,
                                                 const double sigma_d,
                                                 const double sigma_r,
                                                 const uint32_t n);

  static Mat3d quantizeColors(const Mat3d& imageInLab);

  static Mat3d thresholdingXDoG(const Mat3d response);

 private:
  // orientation aligned bilateral filter
  double _oabfSigma_d      = 3.0;
  double _oabfSigma_r      = 4.25;
  uint32_t _oabfIterations = 5;

  Mat3d overlay(const Mat3d& edge, const Mat3d& image) const;
};

}  // namespace painty
