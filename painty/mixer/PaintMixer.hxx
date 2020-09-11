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

  Palette mixFromInputPicture(const Mat<vec3>& sRGBPicture,
                              uint32_t count) const;

  PaintCoeff mixSinglePaint(const std::vector<CoeffPrecision>& weights) const;

  std::vector<CoeffPrecision> getWeightsForMixingTargetPaint(
    const PaintCoeff& paint) const;

  std::vector<CoeffPrecision> getMixtureWeightsForReflectance(
    const vec3& targetReflectance, const vec3& backgroundReflectance,
    double& layerThickness) const;

  const Palette& getUnderlyingPalette() const;
  void setUnderlyingPalette(const Palette& palette);

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
