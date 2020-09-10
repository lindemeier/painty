/**
 * @file PaintCoeff.cxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2019-04-13
 *
 */

#include "painty/mixer/PaintCoeff.hxx"

namespace painty {}  // namespace painty

std::ostream& operator<<(std::ostream& output, const painty::PaintCoeff& v) {
  output << "K(";
  auto i = 0;
  for (; i < painty::CoeffSamplesCount - 1; i++) {
    output << v.K[i] << ", ";
  }
  output << v.K[i] << ")";
  output << "\tS(";
  i = 0;
  for (; i < painty::CoeffSamplesCount - 1; i++) {
    output << v.S[i] << ", ";
  }
  output << v.S[i] << ")";

  return output;
}
