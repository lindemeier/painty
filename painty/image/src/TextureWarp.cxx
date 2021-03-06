/**
 * @file TextureWarp.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-01
 */
#include "painty/image/TextureWarp.hxx"

#include "painty/core/Math.hxx"

painty::TextureWarp::TextureWarp() {}

void painty::TextureWarp::init(const std::vector<painty::vec2>& in,
                               const std::vector<painty::vec2>& out) {
  _in  = in;
  _out = out;
}

/**
 * @brief Warps 2d coordinates at a given position.
 *
 * @param p
 * @return painty::vec2
 */
painty::vec2 painty::TextureWarp::warp(const painty::vec2& p) const {
  return generalizedBarycentricCoordinatesInterpolate(_in, p, _out);
}
