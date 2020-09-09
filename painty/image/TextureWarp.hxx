/**
 * @file TextureWarp.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-01
 *
 */
#pragma once

#include <vector>

#include "painty/core/Vec.hxx"

namespace painty {
class TextureWarp final {
 public:
  TextureWarp();

  void init(const std::vector<vec2>& in, const std::vector<vec2>& out);

  vec2 warp(const vec2& p) const;

 private:
  std::vector<vec2> _in;
  std::vector<vec2> _out;
};
}  // namespace painty
