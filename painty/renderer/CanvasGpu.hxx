/**
 * @file CanvasGpu.h
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-10-18
 *
 */
#pragma once

#include "painty/core/Vec.hxx"
#include "painty/image/Mat.hxx"
#include "painty/renderer/PaintLayerGpu.hxx"

namespace painty {
class CanvasGpu final {
 public:
  CanvasGpu(const Size& size);

  auto getSize() const -> const Size&;

  void clear();

  auto getPaintLayer() -> PaintLayerGpu&;

  auto getComposition() -> Mat3d;

 private:
  Size _size;

  PaintLayerGpu _paintLayer;

  GpuMat<vec4f> _r0_substrate;
};
}  // namespace painty
