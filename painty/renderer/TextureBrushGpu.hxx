/**
 * @file TextureBrushGpu.hxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-10-07
 *
 */
#pragma once

#include "painty/core/Vec.hxx"
#include "painty/renderer/BrushBase.hxx"
#include "painty/renderer/BrushStrokeSample.hxx"
#include "painty/renderer/CanvasGpu.hxx"
#include "painty/renderer/TextureBrushDictionary.hxx"
#include "prgl/FrameBufferObject.hxx"
#include "prgl/GlslComputeShader.hxx"
#include "prgl/GlslRenderingPipelineProgram.hxx"
#include "prgl/Window.hxx"

namespace painty {
class TextureBrushGpu final : public BrushBase<vec3> {
 public:
  TextureBrushGpu();

  void setRadius(const double) override;

  void dip(const std::array<vec3, 2UL>&) override;

  void paintStroke(const std::vector<vec2>& path,
                   Canvas<vec3>& canvas) override;

  void paintStroke(const std::vector<vec2>& path, CanvasGpu& canvas);

  void enableSmudge(bool enable);

 private:
  void generateWarpedTexture(const std::vector<vec2>& path, const Size& size);

  double _radius = 0.0;

  std::array<vec3, 2UL> _paintStored;

  TextureBrushDictionary _textureBrushDictionary;

  std::shared_ptr<prgl::GlslRenderingPipelineProgram> _shaderWarp = nullptr;

  std::shared_ptr<prgl::GlslComputeShader> _shaderImprint         = nullptr;

  std::shared_ptr<prgl::Texture2d> _warpedBrushTexture            = nullptr;

  std::shared_ptr<prgl::FrameBufferObject> _warpedBrushTextureFbo = nullptr;

  std::shared_ptr<prgl::GlslComputeShader> _shaderClearBrushTexture = nullptr;

  bool _smudge = false;
};
}  // namespace painty
