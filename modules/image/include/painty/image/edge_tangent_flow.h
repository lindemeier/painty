/**
 * @file edge_tangent_flow.h
 * @author Thomas Lindemeier
 * @brief
 *
 * @date 2020-07-31
 *
 */
#pragma once

#include "painty/core/vec.h"
#include "painty/image/mat.h"

namespace painty {
namespace tensor {

double GetMinEigenvalue(const vec3& tensor);

double GetMaxEigenvalue(const vec3& tensor);

vec2 GetMinEigenVector(const vec3& tensor);

vec2 GetMaxEigenvector(const vec3& tensor);

Mat3d ComputeTensors(const Mat3d& image, const Mat1d& mask, double innerSigma,
                     double outerSigma, const double spatialSigma,
                     const double colorSigma);

}  // namespace tensor

Mat2d ComputeEdgeTangentFlow(const Mat3d& structureTensorField);

Mat1d lineIntegralConv(const Mat2d& etf, const double sigmaL);

}  // namespace painty
