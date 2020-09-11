/**
 * @file SbrPainter.cxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-09-11
 *
 */
#include "painty/sbr/SbrPainter.hxx"

#include "painty/core/Spline.hxx"

namespace painty {

SbrPainter::SbrPainter(const std::shared_ptr<Canvas<vec3>>& canvasPtr,
                       const Palette& palette)
    : _mixer(palette),
      _canvasPtr(canvasPtr),
      _brush(0.0) {}

void SbrPainter::setBrushRadius(const double radius) {
  _brush.setRadius(radius);
}

void SbrPainter::dipBrush(const std::array<vec3, 2UL>& paint) {
  _brush.dip(paint);
}

void SbrPainter::paintStroke(const std::vector<vec2>& path) {
  // /**
  //   * @author Zingl Alois
  //   * @date 22.08.2016
  //   * @version 1.2
  //   */
  // const auto bresenham = [](vec2i p0, vec2i p1) -> std::vector<vec2i> {
  //   std::vector<vec2i> points;
  //   int32_t dx = std::abs(p1[0U] - p0[0U]), sx = p0[0U] < p1[0U] ? 1 : -1;
  //   int32_t dy = -std::abs(p1[1U] - p0[1U]), sy = p0[1U] < p1[1U] ? 1 : -1;
  //   int32_t err = dx + dy, e2;

  //   for (;;) {
  //     vec2i p = p0;
  //     if (points.empty() || (points.back() != p)) {
  //       points.push_back(p);
  //     }
  //     if ((p0[0U] == p1[0U]) && (p0[1U] == p1[1U])) {
  //       break;
  //     }
  //     e2 = 2 * err;
  //     if (e2 >= dy) {
  //       err += dy;
  //       p0[0U] += sx;
  //     }
  //     if (e2 <= dx) {
  //       err += dx;
  //       p0[1U] += sy;
  //     }
  //   }
  //   return points;
  // };

  // for (auto i = 1UL; (i < path.size()); i++) {
  //   const auto p0 = path[i - 1UL].cast<int32_t>();
  //   const auto p1 = path[i].cast<int32_t>();

  //   const auto sampled_path = bresenham(p0, p1);

  //   for (const auto& p : sampled_path) {
  //     // TODO implement theta
  //     _brush.imprint(p.cast<double>(), 0.0, *_canvasPtr);
  //   }
  // }
  for (auto i = 0UL; (i < (path.size() - 1UL)); i++) {
    const auto p_pre  = path[std::max(i - 1UL, 0UL)];
    const auto p_0    = path[i];
    const auto p_1    = path[i + 1UL];
    const auto p_next = path[std::min(i + 2UL, path.size() - 1UL)];

    const auto dist = (p_1 - p_0).norm();

    for (int32_t pd = 1; pd <= static_cast<int32_t>(dist); pd++) {
      const auto t = static_cast<double>(pd) / dist;
      // TODO implement theta
      const auto dir = painty::CubicDerivativeFirst(p_pre, p_0, p_1, p_next, t);
      _brush.imprint(painty::Cubic(p_pre, p_0, p_1, p_next, t),
                     std::atan2(dir[1U], dir[0U]), *_canvasPtr);
    }
  }
}

}  // namespace painty
