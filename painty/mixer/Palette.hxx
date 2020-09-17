/**
 * @file Palette.hxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2019-04-13
 *
 */
#pragma once

#include <vector>

#include "painty/core/Vec.hxx"
#include "painty/image/Mat.hxx"
#include "painty/mixer/PaintCoeff.hxx"

namespace painty {

using Palette = std::vector<PaintCoeff>;

auto getThinningMedium() -> PaintCoeff;

/**
  * @brief Estimate thickness map using painted examples consisting of background reflectance R0 and resulting reflectance R1.
  * Implementation of the algorithm for paint coefficient measuring proposed by:
  *  T. Lindemeier, J. M. Gülzow, and O. Deussen. 2018. Painterly rendering using limited paint color palettes.
  *  In Proceedings of the Conference on Vision, Modeling, and Visualization (EG VMV '18).
  *  Eurographics Association, Goslar Germany, Germany, 135-145. DOI: https://doi.org/10.2312/vmv.20181263
  *
  * @param R0 contains all color samples before applying the paints (linear RGB)
  * @param R_target contains all color samples after applying the paints (linear RGB)
  * @param masks vector of masks to group all paints and mask out invalid samples
  * @param thicknessMap output of the estimated thickness values
  * @param applied output, applied the paint to R0 result
  * @return the extracted thickness map palette
  */
auto extractThickness(const Mat3d& R0, const Mat3d& R_target,
                      const std::vector<Mat1d>& masks, const Palette& palette)
  -> Mat1d;

/**
  * @brief Estimate absorption and scattering using painted examples consisting of background reflectance R0 and resulting reflectance R1.
  * Implementation of the algorithm for paint coefficient measuring proposed by:
  *  T. Lindemeier, J. M. Gülzow, and O. Deussen. 2018. Painterly rendering using limited paint color palettes.
  *  In Proceedings of the Conference on Vision, Modeling, and Visualization (EG VMV '18).
  *  Eurographics Association, Goslar Germany, Germany, 135-145. DOI: https://doi.org/10.2312/vmv.20181263
  *
  * @param R0 contains all color samples before applying the paints (linear RGB)
  * @param R_target contains all color samples after applying the paints (linear RGB)
  * @param masks vector of masks to group all paints and mask out invalid samples
  * @param thicknessMap output of the estimated thickness values
  * @param applied output, applied the paint to R0 result
  * @param estimateSingleThickness if true only one single thickness value per paint is estimated
  * @return the measured palette
  */
auto createPaletteFromReflectanceData(const Mat3d& R0, const Mat3d& R_target,
                                      const std::vector<Mat1d>& masks,
                                      Mat1d& thicknessMap,
                                      bool estimateSingleThickness) -> Palette;

}  // namespace painty
