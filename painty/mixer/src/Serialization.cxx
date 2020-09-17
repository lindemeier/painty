/**
 * @file Serialization.cxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2019-04-13
 *
 */

#include "painty/mixer/Serialization.hxx"

#include <istream>
#include <ostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "nlohmann/json.hpp"
#pragma clang diagnostic pop
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "painty/core/Color.hxx"
#include "painty/core/KubelkaMunk.hxx"

namespace painty {

static void to_json(nlohmann::json& j, const PaintCoeff& p) {
  j["K"] = {p.K[0U], p.K[1U], p.K[2U]};
  j["S"] = {p.S[0U], p.S[1U], p.S[2U]};
}

static void from_json(const nlohmann::json& j, PaintCoeff& p) {
  const auto K = j.at("K");
  const auto S = j.at("S");
  p.K          = {K.at(0U), K.at(1U), K.at(2U)};
  p.S          = {S.at(0U), S.at(1U), S.at(2U)};
}

void LoadPalette(std::istream& stream, Palette& palette) {
  nlohmann::json j;
  stream >> j;
  for (auto& element : j) {
    palette.push_back(element.get<painty::PaintCoeff>());
  }
}

void SavePalette(std::ostream& stream, const Palette& palette) {
  auto jsonObjects = nlohmann::json::array();
  for (const auto& coeff : palette) {
    jsonObjects.push_back(coeff);
  }
  nlohmann::json j = jsonObjects;
  stream << j;
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
  Mat<vec3> paletteImage(200, static_cast<int32_t>(palette.size()) * 100);

  ColorConverter<double> converter;

  const vec3 black = {0., 0., 0.};
  const vec3 white = {1., 1., 1.};

  for (int32_t c = 0; c < static_cast<int32_t>(palette.size()); c++) {
    const auto& paint = palette[static_cast<size_t>(c)];

    vec3 colorOnBlackRGB =
      ComputeReflectance(paint.K, paint.S, black, appliedThickness);
    vec3 colorOnWhiteRGB =
      ComputeReflectance(paint.K, paint.S, white, appliedThickness);

    vec3 colorOnBlack, colorOnWhite;
    converter.rgb2srgb(colorOnBlackRGB, colorOnBlack);
    converter.rgb2srgb(colorOnWhiteRGB, colorOnWhite);

    const auto h = static_cast<double>(paletteImage.rows);

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
      st = static_cast<int32_t>(0.1 * h);

      for (int32_t y = st; y < 0.4 * h; y++) {
        paletteImage(y, x)[0] = colorOnWhite[0];
        paletteImage(y, x)[1] = colorOnWhite[1];
        paletteImage(y, x)[2] = colorOnWhite[2];
      }
      st = static_cast<int32_t>(0.4 * h);

      for (int32_t y = st; y < 0.5 * h; y++) {
        paletteImage(y, x)[0] = 1.0;
        paletteImage(y, x)[1] = 1.0;
        paletteImage(y, x)[2] = 1.0;
      }
      st = static_cast<int32_t>(0.5 * h);

      for (int32_t y = st; y < 0.6 * h; y++) {
        paletteImage(y, x)[0] = 0.0;
        paletteImage(y, x)[1] = 0.0;
        paletteImage(y, x)[2] = 0.0;
      }
      st = static_cast<int32_t>(0.6 * h);

      for (int32_t y = st; y < 0.9 * h; y++) {
        paletteImage(y, x)[0] = colorOnBlack[0];
        paletteImage(y, x)[1] = colorOnBlack[1];
        paletteImage(y, x)[2] = colorOnBlack[2];
      }
      st = static_cast<int32_t>(0.9 * h);

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
