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
#include "prgl/Window.hxx"

namespace painty {
class TextureBrushGpu final : public BrushBase<vec3> {
 public:
  TextureBrushGpu(const std::shared_ptr<prgl::Window>& window);

  void setRadius(const double) override;

  void dip(const std::array<vec3, 2UL>&) override;

  void paintStroke(const std::vector<vec2>& path,
                   Canvas<vec3>& canvas) override;

  void paintStroke(const std::vector<vec2>& path, CanvasGpu& canvas);

 private:
  auto generateWarpedTexture(const std::vector<vec2>& path,
                             const vec2& boundMin, const vec2& boundMax) const
    -> std::shared_ptr<prgl::Texture2d>;

  std::shared_ptr<prgl::Window> _glWindow = nullptr;

  double _radius = 0.0;

  std::array<vec3, 2UL> _paintStored;

  BrushStrokeSample _brushStrokeSample;

  TextureBrushDictionary _textureBrushDictionary;
};
}  // namespace painty
