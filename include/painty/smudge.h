/**
 * @file smudge.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-28
 *
 */
#ifndef PAINTY_SMUDGE_H
#define PAINTY_SMUDGE_H

#include <painty/paint_layer.h>
#include <painty/spline.h>

namespace painty {
template <class vector_type>
class Smudge final {
  using T                 = typename DataType<vector_type>::channel_type;
  static constexpr auto N = DataType<vector_type>::dim;

 public:
  Smudge(const uint32_t size)
      : _maxSize(size),
        _pickupMapSrc(_maxSize, _maxSize),
        _pickupMapDst(_maxSize, _maxSize) {
    if ((_maxSize % 2) == 0) {
      _maxSize++;
    }
    clean();
  }

  void clean() {
    _pickupMapSrc.clear();
    _pickupMapDst.clear();
    _currentRotation = 0.0;
  }

  void smudge(Canvas<vector_type>& canvas, const vec2& boundMin,
              SplineEval<std::vector<vec2>::const_iterator>& spineSpline,
              T length, const Mat<T>& thicknessMap) {
    // rasterizing spline brute force, by step sampling the spline and using bresenham to connect the sample points
    std::vector<vec<int32_t, 2>> rasteredPoints;
    for (double u = 0.0; u < 1.0; u += 1. / length) {
      const auto p0 = spineSpline.cubic(u);
      const auto p1 = spineSpline.cubic(u + 1. / length);
      bresenham(rasteredPoints, static_cast<int32_t>(p0[0]),
                static_cast<int32_t>(p0[1]), static_cast<int32_t>(p1[0]),
                static_cast<int32_t>(p1[1]));
    }

    auto maxD = 0.0;
    for (const auto& m : thicknessMap.getData()) {
      maxD = std::max(m, maxD);
    }
    if (maxD <= 0.) {
      return;
    }

    for (size_t i = 0; i < rasteredPoints.size(); ++i) {
      vec2 center;
      center[0] = rasteredPoints[i][0] + 0.5;
      center[1] = rasteredPoints[i][1] + 0.5;
      auto u    = static_cast<double>(i) / (rasteredPoints.size() - 1);
      // spine tangent vector
      vec2 t = spineSpline.linearDerivative(u).normalized();
      updateOrientation(t);

      auto radius = _maxSize * 0.5;

      const auto roi_x = center[0] - _pickupMapDst.getCols() / 2.0;
      const auto roi_y = center[1] - _pickupMapDst.getRows() / 2.0;

      for (auto x = 0U; x < _pickupMapDst.getCols(); x++) {
        for (auto y = 0U; y < _pickupMapDst.getRows(); y++) {
          if (x < 0U || x >= _pickupMapDst.getCols() || y < 0U ||
              y >= _pickupMapDst.getRows())
            continue;
          vec<int32_t, 2> sp(x, y);
          vec<int32_t, 2> cp(x + roi_x, y + roi_y);
          vec<int32_t, 2> tp(static_cast<int32_t>(cp[0] - boundMin[0U]),
                             static_cast<int32_t>(cp[1] - boundMin[1U]));

          const auto dist = (center - cp.cast<double>()).norm();
          if (dist > radius)
            continue;

          const auto D = thicknessMap(static_cast<uint32_t>(tp[1]),
                                      static_cast<uint32_t>(tp[0]));
          if (D <= 0.0) {
            continue;
          }

          const auto cV = canvas.getPaintLayer().getV_buffer()(
            static_cast<uint32_t>(cp[1]), static_cast<uint32_t>(cp[0]));
          const auto pV = _pickupMapDst.getV_buffer()(
            static_cast<uint32_t>(sp[1]), static_cast<uint32_t>(sp[0]));

          // paint pickup from canvas
          const auto cVl = cV * _ratePickup * D / maxD;
          const auto cVr = cV - cVl;

          // paint distributed to canvas
          const auto pVl = pV * _rateRelease * D / maxD;
          const auto pVr = pV - pVl;

          const auto canvasK = canvas.getPaintLayer().getK_buffer()(
            static_cast<uint32_t>(cp[1]), static_cast<uint32_t>(cp[0]));
          const auto canvasS = canvas.getPaintLayer().getS_buffer()(
            static_cast<uint32_t>(cp[1]), static_cast<uint32_t>(cp[0]));

          const auto pickK = _pickupMapDst.getK_buffer()(
            static_cast<uint32_t>(sp[1]), static_cast<uint32_t>(sp[0]));
          const auto pickS = _pickupMapDst.getS_buffer()(
            static_cast<uint32_t>(sp[1]), static_cast<uint32_t>(sp[0]));

          // pickup from canvas
          const auto pVnew = pVr + cVl;
          if (pVnew > 0.0) {
            const auto pVnew_ = 1. / pVnew;
            _pickupMapDst.getK_buffer()(static_cast<uint32_t>(sp[1]),
                                        static_cast<uint32_t>(sp[0])) =
              pVnew_ * (pVr * pickK + cVl * canvasK);
            _pickupMapDst.getS_buffer()(static_cast<uint32_t>(sp[1]),
                                        static_cast<uint32_t>(sp[0])) =
              pVnew_ * (pVr * pickS + cVl * canvasS);
            _pickupMapDst.getV_buffer()(static_cast<uint32_t>(sp[1]),
                                        static_cast<uint32_t>(sp[0])) =
              std::max(pVnew, 0.0);
          }

          // deposition of paint
          const auto cVnew = cVr + pVl;
          if (cVnew > 0.0) {
            const auto cVnew_ = 1. / cVnew;
            canvas.getPaintLayer().getK_buffer()(static_cast<uint32_t>(cp[1]),
                                                 static_cast<uint32_t>(cp[0])) =
              cVnew_ * (cVr * canvasK + pVl * pickK);
            canvas.getPaintLayer().getS_buffer()(static_cast<uint32_t>(cp[1]),
                                                 static_cast<uint32_t>(cp[0])) =
              cVnew_ * (cVr * canvasS + pVl * pickS);
            canvas.getPaintLayer().getV_buffer()(static_cast<uint32_t>(cp[1]),
                                                 static_cast<uint32_t>(cp[0])) =
              std::max(cVnew, 0.0);
          }
        }
      }
    }
  }

 private:
  uint32_t _maxSize = 0;

  PaintLayer<vector_type> _pickupMapSrc;

  PaintLayer<vector_type> _pickupMapDst;

  T _currentRotation = 0.0;

  T _rateRelease = 0.1;

  T _ratePickup = 0.1;

  void updateOrientation(const vec2& heading) {
    const auto theta =
      std::atan2(heading[1], heading[0]);  // get rotation around tool
    const auto dtheta = normalizeAngle(theta - _currentRotation);

    _currentRotation = theta;

    _pickupMapDst.copyTo(_pickupMapSrc);

    vec2 center = {_maxSize / 2.f, _maxSize / 2.f};

    // update pickupmap
    for (auto x = 0U; x < _maxSize; x++) {
      for (auto y = 0U; y < _maxSize; y++) {
        vec<uint32_t, 2UL> destCoords(x, y);
        vec2 pickupPos = rotate(destCoords, dtheta, center);
        if (pickupPos[0] < 0 || pickupPos[1] < 0 || pickupPos[0] >= _maxSize ||
            pickupPos[1] >= _maxSize) {
          _pickupMapDst.getK_buffer()(destCoords[1], destCoords[0]) =
            _pickupMapSrc.getK_buffer()(destCoords[1], destCoords[0]);
          _pickupMapDst.getS_buffer()(destCoords[1], destCoords[0]) =
            _pickupMapSrc.getS_buffer()(destCoords[1], destCoords[0]);
          _pickupMapDst.getV_buffer()(destCoords[1], destCoords[0]) =
            _pickupMapSrc.getV_buffer()(destCoords[1], destCoords[0]);
        } else {
          const auto pickupV = _pickupMapSrc.getV_buffer()(pickupPos);
          const auto pickupK = _pickupMapSrc.getK_buffer()(pickupPos);
          const auto pickupS = _pickupMapSrc.getS_buffer()(pickupPos);

          _pickupMapDst.getV_buffer()(destCoords[1], destCoords[0]) = pickupV;
          _pickupMapDst.getK_buffer()(destCoords[1], destCoords[0]) = pickupK;
          _pickupMapDst.getS_buffer()(destCoords[1], destCoords[0]) = pickupS;
        }
      }
    }
  }

  T normalizeAngle(T angle) {
    constexpr auto HALF_PI = 0.5 * painty::PI;
    auto newAngle          = angle;
    while (newAngle <= -HALF_PI) {
      newAngle += painty::PI;
    }
    while (newAngle > HALF_PI) {
      newAngle -= painty::PI;
    }
    return newAngle;
  }

  vec2 rotate(const vec<uint32_t, 2UL>& p, T a, const vec2& center) {
    vec2 p_;

    auto s = std::sin(a);
    auto c = std::cos(a);

    p_[0] = p[0] - center[0];
    p_[1] = p[1] - center[1];

    auto xnew = p_[0] * c - p_[1] * s;
    auto ynew = p_[0] * s + p_[1] * c;

    p_[0] = xnew + center[0];
    p_[1] = ynew + center[1];
    return p_;
  }

  void bresenham(std::vector<vec<int32_t, 2UL>>& points, int32_t x0, int32_t y0,
                 int32_t x1, int32_t y1) {
    /**
     * @author Zingl Alois
     * @date 22.08.2016
     * @version 1.2
     */
    int32_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int32_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int32_t err = dx + dy, e2; /* error value e_xy */

    for (;;) { /* loop */
      vec<int32_t, 2UL> p;
      p << x0, y0;
      if (points.empty() || points.back() != vec<int32_t, 2UL>(p[0], p[1])) {
        points.push_back(vec<int32_t, 2UL>(p[0], p[1]));
      }
      if (x0 == x1 && y0 == y1)
        break;
      e2 = 2 * err;
      if (e2 >= dy) {
        err += dy;
        x0 += sx;
      } /* e_xy+e_x > 0 */
      if (e2 <= dx) {
        err += dx;
        y0 += sy;
      } /* e_xy+e_y < 0 */
    }
  }
};
}  // namespace painty

#endif  // PAINTY_SMUDGE_H
