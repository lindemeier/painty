/**
 * @file BrushBase.hxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-09-14
 *
 */
#pragma once

#include <array>

#include "painty/core/Vec.hxx"
#include "painty/renderer/Canvas.hxx"

namespace painty {
template <class vector_type>
class BrushBase {
 public:
  using T                 = typename DataType<vector_type>::channel_type;
  static constexpr auto N = DataType<vector_type>::dim;

  virtual ~BrushBase() = default;

  virtual void setRadius(const double radius) = 0;

  virtual void dip(const std::array<vector_type, 2UL>& paint) = 0;

  virtual void paintStroke(const std::vector<vec2>& path,
                           Canvas<vector_type>& canvas) = 0;
};
}  // namespace painty
