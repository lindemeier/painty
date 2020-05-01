/**
 * @file texture_warp.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-01
 */
#include <painty/texture_warp.h>

#include <painty/math.h>

painty::TextureWarp::TextureWarp(const std::vector<painty::vec2>& in, const std::vector<painty::vec2>& out)
  : _in(in), _out(out)
{
}

/**
 * @brief Warps 2d coordinates at a given position.
 *
 * @param p
 * @return painty::vec2
 */
painty::vec2 painty::TextureWarp::warp(const painty::vec2& p)
{
  return generalizedBarycentricCoordinatesInterpolate(_in, p, _out);
}
