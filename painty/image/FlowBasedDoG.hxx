/**
 * @file FlowBasedDoG.h
 * @author Thomas Lindemeier
 * @date 2020-08-27
 *
 *
 */
#pragma once
#include "painty/image/Mat.hxx"

namespace painty {
/**
 * @brief Implementation of the paper:
 * Kyprianidis, J. E., and Kang, H. Image and video abstraction by coherence-enhancing filtering. Computer Graphics Forum 30, 2 (Apr. 2011), 593–602
 *
 */
class FlowBasedDoG {
 public:
  Mat3d execute(const Mat3d& rgbLinear) const;

  static Mat3d filterBilateralOrientationAligned(const Mat3d& imageInLab,
                                                 const Mat2d& edgeTangentFlow,
                                                 double sigma_d, double sigma_r,
                                                 uint32_t n);

  static Mat3d quantizeColors(const Mat3d& imageInLab, double phi_q,
                              uint32_t nbins);

  static Mat1d thresholdingXDoG(const Mat1d& response, double xdogParamEps,
                                double xdogParamPhi);

  static Mat1d filterFlowBasedDoG(const Mat1d& img, const Mat2d& tfm,
                                  double sigma_e, double sigma_r, double tau,
                                  double sigmaSmoothing);

 private:
  // orientation aligned bilateral filter
  double _oabfSigma_d      = 3.0;
  double _oabfSigma_r      = 4.25;
  uint32_t _oabfIterations = 5;

  // color quantization
  double _phi_q   = 3.4;
  uint32_t _nbins = 6;

  // standard deviation of the Gaussian blur
  double _xdogParamSigma = 3.0;
  // Differences of Gaussians factor
  double _xdogParamKappa = 1.6;
  // shifts the detection threshold, thereby controlling sensitivity (albeit on
  // an inverted scale: Smaller values make the edge detection more
  // sensitive, while large values decrease detection sensitivity).
  double _xdogParamEps = 0.0;
  // changes the relative weighting between the larger and
  // smaller Gaussians, thereby affecting the tone-mapping response of
  // the operator.
  double _xdogParamTau = 0.99;
  //creates an adjustable
  //soft ramp between the edge and non-edge values, with parameter φ
  //controlling the steepness of this transition
  double _xdogParamPhi            = 2.0;
  double _xdogParamSmoothingSigma = 3.0;

  double _tensorOuterSigma = 3.0;

  static Mat3d overlay(const Mat1d& edge, const Mat3d& image);
};

}  // namespace painty
