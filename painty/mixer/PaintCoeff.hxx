/**
 * @file PaintCoeff.hxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2019-04-13
 *
 */
#ifndef PAINT_MIXER_PAINT_COEFF_H
#define PAINT_MIXER_PAINT_COEFF_H

#include <array>
#include <iostream>

#include "painty/core/Vec.hxx"

namespace painty {

constexpr auto CoeffSamplesCount = 3;
using CoeffPrecision             = double;

/**
 * @brief Represents Kubelka-Munk coefficients.
 *
 */
struct PaintCoeff {
  using VecType = vec<CoeffPrecision, CoeffSamplesCount>;

  /**
   * @brief Absorption
   */
  VecType K;
  /**
   * @brief Scattering
   */
  VecType S;
};

}  // namespace painty

std::ostream& operator<<(std::ostream& output, const painty::PaintCoeff& v);

#endif  // PAINT_MIXER_PAINT_COEFF_H
