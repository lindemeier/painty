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

  void composeOnto(GpuMat<vec3f>& R0) const;

 private:
  Size _size;

  GpuMat<vec3f> _K;
  GpuMat<vec3f> _S;
  GpuMat<float> _V;
};
}  // namespace painty
