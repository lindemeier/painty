/**
 * @file Serialization.hxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2019-04-13
 *
 */
#ifndef PAINT_MIXER_SERIALIZATTION_H
#define PAINT_MIXER_SERIALIZATTION_H

#include <filesystem>
#include <iostream>

#include "painty/image/Mat.hxx"
#include "painty/mixer/PaintCoeff.hxx"
#include "painty/mixer/Palette.hxx"

namespace painty {

void LoadPalette(std::istream& stream, Palette& palette);

void SavePalette(std::ostream& stream, const Palette& palette);

Mat<vec3> VisualizePalette(const Palette& palette,
                           const double appliedThickness);

}  // namespace painty

#endif  // PAINT_MIXER_SERIALIZATTION_H
