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

  auto getK() -> GpuMat<vec4f>&;
  auto getS() -> GpuMat<vec4f>&;

 private:
  Size _size;

  /**
   * @brief Need to use 4 channels due to GLSL formats.
   * Storing volume as alpha value in both buffers.
   *
   */
  GpuMat<vec4f> _K;
  GpuMat<vec4f> _S;
};
}  // namespace painty
