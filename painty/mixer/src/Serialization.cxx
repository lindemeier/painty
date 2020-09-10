/**
 * @file Serialization.cxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2019-04-13
 *
 */

#include "painty/mixer/Serialization.hxx"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "painty/core/Color.hxx"
#include "painty/core/KubelkaMunk.hxx"
#include "painty/image/Mat.hxx"

namespace painty {
void LoadPalette(std::istream& stream, Palette& palette) {
  cereal::JSONInputArchive ar(stream);
  ar(cereal::make_nvp("palette", palette));
}

void SavePalette(std::ostream& stream, const Palette& palette) {
  cereal::JSONOutputArchive ar(stream);
  ar(cereal::make_nvp("palette", palette));
}

/**
 * @brief Visualize the palette colors by applying them onto black and white
 * backgrounds.
 *
 * @param palette
 * @param appliedThickness
 *
 * @return Mat<vec3>
 */
Mat<vec3> VisualizePalette(const Palette& palette,
                           const double appliedThickness) {
  Mat<vec3> paletteImage(200, palette.size() * 100);

  ColorConverter<double> converter;

  const vec3 black = {0., 0., 0.};
  const vec3 white = {1., 1., 1.};

  for (int32_t c = 0; c < static_cast<int32_t>(palette.size()); c++) {
    const auto& paint = palette[c];

    vec3 colorOnBlackRGB =
      ComputeReflectance(paint.K, paint.S, black, appliedThickness);
    vec3 colorOnWhiteRGB =
      ComputeReflectance(paint.K, paint.S, white, appliedThickness);

    vec3 colorOnBlack, colorOnWhite;
    converter.rgb2srgb(colorOnBlackRGB, colorOnBlack);
    converter.rgb2srgb(colorOnWhiteRGB, colorOnWhite);

    const int32_t h = paletteImage.rows;

    for (int32_t x =
           c * (paletteImage.cols / static_cast<int32_t>(palette.size()));
         x <
         (c + 1) * (paletteImage.cols / static_cast<int32_t>(palette.size()));
         x++) {
      int32_t st = 0;

      for (int32_t y = st; y < 0.1 * h; y++) {
        paletteImage(y, x)[0] = 1.0;
        paletteImage(y, x)[1] = 1.0;
        paletteImage(y, x)[2] = 1.0;
      }
      st = 0.1 * h;

      for (int32_t y = st; y < 0.4 * h; y++) {
        paletteImage(y, x)[0] = colorOnWhite[0];
        paletteImage(y, x)[1] = colorOnWhite[1];
        paletteImage(y, x)[2] = colorOnWhite[2];
      }
      st = 0.4 * h;

      for (int32_t y = st; y < 0.5 * h; y++) {
        paletteImage(y, x)[0] = 1.0;
        paletteImage(y, x)[1] = 1.0;
        paletteImage(y, x)[2] = 1.0;
      }
      st = 0.5 * h;

      for (int32_t y = st; y < 0.6 * h; y++) {
        paletteImage(y, x)[0] = 0.0;
        paletteImage(y, x)[1] = 0.0;
        paletteImage(y, x)[2] = 0.0;
      }
      st = 0.6 * h;

      for (int32_t y = st; y < 0.9 * h; y++) {
        paletteImage(y, x)[0] = colorOnBlack[0];
        paletteImage(y, x)[1] = colorOnBlack[1];
        paletteImage(y, x)[2] = colorOnBlack[2];
      }
      st = 0.9 * h;

      for (int32_t y = st; y < h; y++) {
        paletteImage(y, x)[0] = 0.0;
        paletteImage(y, x)[1] = 0.0;
        paletteImage(y, x)[2] = 0.0;
      }
    }
  }
  return paletteImage;
}

}  // namespace painty
