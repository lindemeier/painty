/**
 * @file SbrPainter.hxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-09-11
 *
 */
#pragma once

#include "painty/mixer/PaintMixer.hxx"
#include "painty/mixer/Palette.hxx"
#include "painty/renderer/Canvas.hxx"
#include "painty/renderer/FootprintBrush.hxx"
#include "painty/renderer/TextureBrush.hxx"

namespace painty {
class SbrPainterBase {
 public:
  SbrPainterBase();

  virtual ~SbrPainterBase();

  virtual void setBrushRadius(const double radius) = 0;

  virtual void paintStroke(const std::vector<vec2>& path,
                           Canvas<vec3>& canvas) = 0;

  virtual void dipBrush(const std::array<vec3, 2UL>& paint) = 0;
};

class SbrPainterTextureBrush final : public SbrPainterBase {
 public:
  SbrPainterTextureBrush();

  ~SbrPainterTextureBrush() override = default;

  void setBrushRadius(const double radius) override;
  void paintStroke(const std::vector<vec2>& path,
                   Canvas<vec3>& canvas) override;
  void dipBrush(const std::array<vec3, 2UL>& paint) override;

 private:
  std::unique_ptr<TextureBrush<vec3>> _brushPtr = nullptr;
};

class SbrPainterFootprintBrush final : public SbrPainterBase {
 public:
  SbrPainterFootprintBrush();

  ~SbrPainterFootprintBrush() override = default;

  void setBrushRadius(const double radius) override;
  void dipBrush(const std::array<vec3, 2UL>& paint) override;
  void paintStroke(const std::vector<vec2>& path,
                   Canvas<vec3>& canvas) override;

 private:
  std::unique_ptr<FootprintBrush<vec3>> _brushPtr = nullptr;
};
}  // namespace painty
