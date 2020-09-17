/**
 * @file Smudge.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-28
 *
 */
#pragma once

#include "painty/core/Spline.hxx"
#include "painty/renderer/PaintLayer.hxx"

namespace painty {
template <class vector_type>
class Smudge final {
  using T                 = typename DataType<vector_type>::channel_type;
  static constexpr auto N = DataType<vector_type>::dim;

 public:
  Smudge(const int32_t size)
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
    auto maxD = 0.0;
    for (const auto& m : thicknessMap) {
      maxD = std::max(m, maxD);
    }
    if (maxD <= 0.) {
      return;
    }

    for (double u = 0.0; u <= 1.0; u += 1. / length) {
      const auto center = spineSpline.catmullRom(u);
      // spine tangent vector
      vec2 t = spineSpline.catmullRomDerivativeFirst(u).normalized();
      updateOrientation(t);

      const auto radius = _maxSize * 0.5;

      const int32_t roi_x =
        static_cast<int32_t>(center[0] - _pickupMapDst.getCols() / 2.0);
      const int32_t roi_y =
        static_cast<int32_t>(center[1] - _pickupMapDst.getRows() / 2.0);
      const int32_t cHeight = canvas.getPaintLayer().getV_buffer().rows;
      const int32_t cWidth  = canvas.getPaintLayer().getV_buffer().cols;
      const int32_t pHeight = _pickupMapDst.getRows();
      const int32_t pWidth  = _pickupMapDst.getCols();
      const int32_t yHeight = thicknessMap.rows;
      const int32_t yWidth  = thicknessMap.cols;

      for (auto x = 0; x < _pickupMapDst.getCols(); x++) {
        for (auto y = 0; y < _pickupMapDst.getRows(); y++) {
          vec<int32_t, 2> sp = {x, y};
          vec<int32_t, 2> cp = {x + roi_x, y + roi_y};
          vec<int32_t, 2> tp = {static_cast<int32_t>(cp[0] - boundMin[0U]),
                                static_cast<int32_t>(cp[1] - boundMin[1U])};

          if ((cp[0U] < 0) || (cp[1U] < 0) || (cp[0U] >= cWidth) ||
              (cp[1U] >= cHeight)) {
            continue;
          }
          if ((sp[0U] < 0) || (sp[1U] < 0) || (sp[0U] >= pWidth) ||
              (sp[1U] >= pHeight)) {
            continue;
          }
          if ((tp[0U] < 0) || (tp[1U] < 0) || (tp[0U] >= yWidth) ||
              (tp[1U] >= yHeight)) {
            continue;
          }

          const auto dist = (center - cp.cast<double>()).norm();
          if (dist > radius) {
            continue;
          }

          const auto D = thicknessMap(static_cast<int32_t>(tp[1]),
                                      static_cast<int32_t>(tp[0]));
          if (D <= 0.0) {
            continue;
          }

          const auto cV = canvas.getPaintLayer().getV_buffer()(cp[1], cp[0]);
          const auto pV = _pickupMapDst.getV_buffer()(sp[1], sp[0]);

          // paint pickup from canvas
          const auto cVl = cV * _depositionRate * D / maxD;
          const auto cVr = cV - cVl;

          // paint distributed to canvas
          const auto pVl = pV * _pickupRate * D / maxD;
          const auto pVr = pV - pVl;

          const auto canvasK =
            canvas.getPaintLayer().getK_buffer()(cp[1], cp[0]);
          const auto canvasS =
            canvas.getPaintLayer().getS_buffer()(cp[1], cp[0]);

          const auto pickK = _pickupMapDst.getK_buffer()(sp[1], sp[0]);
          const auto pickS = _pickupMapDst.getS_buffer()(sp[1], sp[0]);

          constexpr auto MinVolume = 0.001;

          // pickup from canvas
          const auto pVnew = pVr + cVl;
          if (pVnew > MinVolume) {
            const auto pVnew_ = 1.0 / pVnew;
            _pickupMapDst.getK_buffer()(sp[1], sp[0]) =
              pVnew_ * (pVr * pickK + cVl * canvasK);
            _pickupMapDst.getS_buffer()(sp[1], sp[0]) =
              pVnew_ * (pVr * pickS + cVl * canvasS);
            _pickupMapDst.getV_buffer()(sp[1], sp[0]) = std::max(pVnew, 0.0);
          }

          // deposition of paint
          const auto cVnew = cVr + pVl;
          if (cVnew > MinVolume) {
            const auto cVnew_ = 1. / cVnew;
            canvas.getPaintLayer().getK_buffer()(cp[1], cp[0]) =
              cVnew_ * (cVr * canvasK + pVl * pickK);
            canvas.getPaintLayer().getS_buffer()(cp[1], cp[0]) =
              cVnew_ * (cVr * canvasS + pVl * pickS);
            canvas.getPaintLayer().getV_buffer()(cp[1], cp[0]) =
              std::max(cVnew, 0.0);
          }
        }
      }
    }
  }

 private:
  int32_t _maxSize = 0;

  PaintLayer<vector_type> _pickupMapSrc;

  PaintLayer<vector_type> _pickupMapDst;

  T _currentRotation = 0.0;

  T _pickupRate = 0.1;

  T _depositionRate = 0.1;

  void updateOrientation(const vec2& heading) {
    const auto theta =
      std::atan2(heading[1], heading[0]);  // get rotation around tool
    const auto dtheta = normalizeAngle(theta - _currentRotation);

    _currentRotation = theta;

    _pickupMapDst.copyTo(_pickupMapSrc);

    vec2 center = {_maxSize / 2.0, _maxSize / 2.0};

    // update pickupmap
    for (auto x = 0; x < _pickupMapDst.getCols(); x++) {
      for (auto y = 0; y < _pickupMapDst.getRows(); y++) {
        vec<int32_t, 2UL> destCoords(x, y);
        vec2 pickupPos = rotate(destCoords, dtheta, center);
        if ((pickupPos[0] < 0) || (pickupPos[1] < 0) ||
            (pickupPos[0] >= _pickupMapDst.getCols()) ||
            (pickupPos[1] >= _pickupMapDst.getRows())) {
          _pickupMapDst.getK_buffer()(destCoords[1], destCoords[0]) =
            _pickupMapSrc.getK_buffer()(destCoords[1], destCoords[0]);
          _pickupMapDst.getS_buffer()(destCoords[1], destCoords[0]) =
            _pickupMapSrc.getS_buffer()(destCoords[1], destCoords[0]);
          _pickupMapDst.getV_buffer()(destCoords[1], destCoords[0]) =
            _pickupMapSrc.getV_buffer()(destCoords[1], destCoords[0]);
        } else {
          const auto pickupV =
            Interpolate(_pickupMapSrc.getV_buffer(), pickupPos);
          const auto pickupK =
            Interpolate(_pickupMapSrc.getK_buffer(), pickupPos);
          const auto pickupS =
            Interpolate(_pickupMapSrc.getS_buffer(), pickupPos);

          _pickupMapDst.getV_buffer()(destCoords[1], destCoords[0]) = pickupV;
          _pickupMapDst.getK_buffer()(destCoords[1], destCoords[0]) = pickupK;
          _pickupMapDst.getS_buffer()(destCoords[1], destCoords[0]) = pickupS;
        }
      }
    }
  }

  T normalizeAngle(T angle) {
    constexpr auto HALF_PI = 0.5 * painty::Pi<double>;
    auto newAngle          = angle;
    while (newAngle <= -HALF_PI) {
      newAngle += painty::Pi<double>;
    }
    while (newAngle > HALF_PI) {
      newAngle -= painty::Pi<double>;
    }
    return newAngle;
  }

  vec2 rotate(const vec<int32_t, 2UL>& p, T a, const vec2& center) {
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
};
}  // namespace painty
