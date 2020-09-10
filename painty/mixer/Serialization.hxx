/**
 * @file Serialization.hxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2019-04-13
 *
 */
#ifndef PAINT_MIXER_SERIALIZATTION_H
#define PAINT_MIXER_SERIALIZATTION_H

#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <filesystem>
#include <iostream>

#include "painty/mixer/PaintCoeff.hxx"
#include "painty/mixer/Palette.hxx"

/**
 * @brief wrap the serialization function in namespac cereal for cereal to find
 * the functions
 *
 */
namespace cereal {
template <typename Archive, typename T, size_t S>
void serialize(Archive& archive, std::array<T, S>& m) {
  cereal::size_type s = S;
  archive(cereal::make_size_tag(s));
  if (s != S) {
    throw std::runtime_error("array has incorrect length");
  }
  for (auto& i : m) {
    archive(i);
  }
}
/**
 * @brief Serialize paint coefficients.
 *
 * @tparam Archive
 * @param ar
 * @param coeff
 */
template <class Archive>
void save(Archive& ar, const PaintMixer::PaintCoeff& coeff) {
  ar(cereal::make_nvp<PaintMixer::PaintCoeff>("K", coeff.K));
  ar(cereal::make_nvp<PaintMixer::PaintCoeff>("S", coeff.S));
}

/**
 * @brief Deserialize paint coefficients.
 *
 * @tparam Archive
 * @param ar
 * @param coeff
 */
template <class Archive>
void load(Archive& ar, PaintMixer::PaintCoeff& coeff) {
  ar(cereal::make_nvp<PaintMixer::PaintCoeff>("K", coeff.K));
  ar(cereal::make_nvp<PaintMixer::PaintCoeff>("S", coeff.S));
}

}  // namespace cereal

namespace painty {

void LoadPalette(std::istream& stream, Palette& palette);

void SavePalette(std::ostream& stream, const Palette& palette);

Mat<vec3> VisualizePalette(const Palette& palette,
                           const double appliedThickness);

}  // namespace painty

#endif  // PAINT_MIXER_SERIALIZATTION_H
