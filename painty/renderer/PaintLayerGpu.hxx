/**
 * @file PaintLayerGpu.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-10-08
 *
 */
#pragma once

#include <stdint.h>

#include <array>

#include "painty/gpu/GpuMat.hxx"

namespace painty {

class PaintLayerGpu final {
 public:
  PaintLayerGpu(const Size& size);

  void clear();

  void composeOnto(GpuMat<vec4f>& R0) const;

 private:
  Size _size;

  /**
   * @brief Need to use 4 channels due to GLSL formats.
   *
   */
  GpuMat<vec4f> _K;
  GpuMat<vec4f> _S;
  GpuMat<float> _V;
};
}  // namespace painty
