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
#include "prgl/GlslComputeShader.hxx"
#include "prgl/Window.hxx"

namespace painty {
class CanvasGpu final {
 public:
  CanvasGpu(const Size& size);
  auto getSize() const -> const Size&;
  void clear();

  auto getPaintLayer() -> PaintLayerGpu&;

  auto getComposed() -> const GpuMat<vec4f>&;

  auto getCompositionLinearRgb() -> Mat3d;

  void dryStep(float step = 0.01F);

 private:
  Size _size;

  PaintLayerGpu _paintLayer;

  GpuMat<vec4f> _r0_substrate;
  GpuMat<vec4f> _r0_substrate_copy_buffer;

  std::shared_ptr<prgl::GlslComputeShader> _dryShader = nullptr;
};
}  // namespace painty
