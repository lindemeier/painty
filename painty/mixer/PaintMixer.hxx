/**
 * @file PaintMixer.hxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2019-04-13
 *
 */
#ifndef PAINT_MIXER_PAINT_MIXER_H
#define PAINT_MIXER_PAINT_MIXER_H

#include <opencv2/core.hpp>

#include "painty/image/Mat.hxx"
#include "painty/mixer/Palette.hxx"

namespace painty {

/**
 * @brief Represents a class that offers paint mix functions.
 *
 */
class PaintMixer {
 public:
  PaintMixer(const Palette& basePalette);

  auto mixFromInputPicture(const Mat<vec3>& sRGBPicture, uint32_t count) const
    -> Palette;

  auto mixSinglePaint(const std::vector<CoeffPrecision>& weights) const
    -> PaintCoeff;

  auto getWeightsForMixingTargetPaint(const PaintCoeff& paint) const
    -> std::vector<CoeffPrecision>;

  auto getMixtureWeightsForReflectance(
    const vec3& targetReflectance, const vec3& backgroundReflectance,
    double& layerThickness) const -> std::vector<CoeffPrecision>;

  auto getUnderlyingPalette() const -> const Palette&;
  void setUnderlyingPalette(const Palette& palette);

  auto mixed(const PaintCoeff& paint, const double paintVolume,
             const PaintCoeff& other, const double otherVolume) -> PaintCoeff;

  auto mixClosestFit(const vec3& R0, const vec3& target) -> PaintCoeff;

 private:
  PaintMixer() = delete;

  /**
   * @brief The underlying collection of base paints that can be used to mix
   * other Paints and Palettes.
   *
   */
  Palette _basePalette;
};

}  // namespace painty

#endif  // PAINT_MIXER_PAINT_MIXER_H
