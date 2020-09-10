/**
 * @file ColorExtraction.hxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2019-04-14
 */
#ifndef PAINT_MIXER_COLOR_EXTRACTION_H
#define PAINT_MIXER_COLOR_EXTRACTION_H

#include "painty/image/Mat.hxx"

namespace painty {

void ExtractColorPaletteAharoni(const Mat<vec3>& sRGB,
                                std::vector<vec3>& linearRGB_colors, uint32_t k);
}  // namespace painty
#endif  // PAINT_MIXER_COLOR_EXTRACTION_H
