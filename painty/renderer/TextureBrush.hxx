/**
 * @file TextureBrush.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-19
 *
 */
#pragma once

#include "painty/core/Spline.hxx"
#include "painty/renderer/BrushBase.hxx"
#include "painty/renderer/BrushStrokeSample.hxx"
#include "painty/renderer/Canvas.hxx"
#include "painty/renderer/Smudge.hxx"

namespace painty {
template <class vector_type>
class TextureBrush final : public BrushBase<vector_type> {
  using T                 = typename BrushBase<vector_type>::T;
  static constexpr auto N = BrushBase<vector_type>::N;

 public:
  TextureBrush(const std::string& sampleDir)
      : _brushStrokeSample(sampleDir),
        _smudge(static_cast<int32_t>(2.0 * _radius)) {
    for (auto& c : _paintStored) {
      c.fill(static_cast<T>(0.1));
    }
  }

  void setRadius(const double radius) override {
    constexpr auto Eps = 0.5;
    if (!fuzzyCompare(_radius, radius, Eps)) {
      _radius = radius;
      if (_useSmudge) {
        _smudge = Smudge<vector_type>(static_cast<int32_t>(2.0 * radius));
      }
    }
  }

  /**
   * @brief Dip the brush in paint paint.
   *
   * @param paint
   */
  void dip(const std::array<vector_type, 2UL>& paint) override {
    _paintStored = paint;
  }

  void paintStroke(const std::vector<vec2>& vertices,
                   Canvas<vector_type>& canvas) override {
    if (vertices.size() < 2UL) {
      return;
    }

    // compute bounding rectangle
    auto boundMin = vertices.front();
    auto boundMax = vertices.front();
    for (const auto& xy : vertices) {
      boundMin[0U] = std::min(boundMin[0U], xy[0U]);
      boundMin[1U] = std::min(boundMin[1U], xy[1U]);
      boundMax[0U] = std::max(boundMax[0U], xy[0U]);
      boundMax[1U] = std::max(boundMax[1U], xy[1U]);
    }
    boundMin[0U] = std::max(boundMin[0U] - _radius, 0.0);
    boundMax[0U] =
      std::min(boundMax[0U] + _radius,
               static_cast<T>(canvas.getPaintLayer().getCols() - 1));
    boundMin[1U] = std::max(boundMin[1U] - _radius, 0.0);
    boundMax[1U] =
      std::min(boundMax[1U] + _radius,
               static_cast<T>(canvas.getPaintLayer().getRows() - 1));

    auto length = 0.0;
    for (auto i = 1UL; i < vertices.size(); ++i) {
      length += (vertices[i] - vertices[i - 1]).norm();
    }

    // spine
    SplineEval<std::vector<vec2>::const_iterator> spineSpline(vertices.cbegin(),
                                                              vertices.cend());

    // construct frames around the vertices
    std::vector<vec2> upCanvasCoordinates;
    upCanvasCoordinates.reserve(vertices.size());
    std::vector<vec2> downCanvasCoordinates;
    downCanvasCoordinates.reserve(vertices.size());

    // frames around the texture
    std::vector<vec2> upUv;
    upUv.reserve(vertices.size());
    std::vector<vec2> downUv;
    downUv.reserve(vertices.size());

    for (auto i = 0U; i < vertices.size(); ++i) {
      T u          = static_cast<T>(i) / static_cast<T>(vertices.size() - 1);
      const auto c = spineSpline.catmullRom(u);

      // spine tangent vector
      auto t = spineSpline.catmullRomDerivativeFirst(u).normalized();

      // compute perpendicular vector to spine
      const vec2 d = {-t[1], t[0]};

      const auto l = c - _radius * d;
      const auto r = c + _radius * d;

      upCanvasCoordinates.push_back(l);
      downCanvasCoordinates.push_back(r);

      constexpr auto uvM = 1.0;
      upUv.push_back({u, -uvM});
      downUv.push_back({u, uvM});
    }
    upCanvasCoordinates.insert(upCanvasCoordinates.begin(),
                               downCanvasCoordinates.rbegin(),
                               downCanvasCoordinates.rend());

    upUv.insert(upUv.begin(), downUv.rbegin(), downUv.rend());

    // canvas coordinates to uv coordinates
    TextureWarp canvas2uv;
    canvas2uv.init(upCanvasCoordinates, upUv);

    const auto now = std::chrono::system_clock::now();

    Mat<T> thicknessMap(static_cast<int32_t>(boundMax[1] - boundMin[1] + 1),
                        static_cast<int32_t>(boundMax[0] - boundMin[0] + 1));
    for (auto& p : thicknessMap) {
      p = static_cast<T>(0.0);
    }
    std::vector<vec<int32_t, 2U>> pixels;
    for (auto x = static_cast<int32_t>(boundMin[0U]);
         x <= static_cast<int32_t>(boundMax[0U]); x++) {
      for (auto y = static_cast<int32_t>(boundMin[1U]);
           y <= static_cast<int32_t>(boundMax[1U]); y++) {
        if ((x < 0) || (x >= canvas.getPaintLayer().getCols()) || (y < 0) ||
            (y >= canvas.getPaintLayer().getRows())) {
          continue;
        }

        // if (!PointInPolyon(upCanvasCoordinates, { static_cast<T>(x), static_cast<T>(y) }))
        // {
        //   continue;
        // }

        // transform canvas coordinates to uv local coordinates
        vec2 canvasUV = canvas2uv.warp({static_cast<T>(x), static_cast<T>(y)});

        // uv not in stroke
        if ((canvasUV[0U] < 0.0) || (canvasUV[0U] > 1.0) ||
            (canvasUV[1U] < -1.0) || (canvasUV[1U] > 1.0)) {
          continue;
        }

        // retrieve the height of the sample at uv
        const auto Vtex = BrushBase<vector_type>::getThicknessScale() *
                          _brushStrokeSample.getSampleAtUV(canvasUV);
        if (Vtex > 0.0) {
          const auto s = x - static_cast<int32_t>(boundMin[0U]);
          const auto t = y - static_cast<int32_t>(boundMin[1U]);
          if ((s >= 0) && (t >= 0) && (s < thicknessMap.cols) &&
              (t < thicknessMap.rows)) {
            canvas.checkDry(x, y, now);
            thicknessMap(t, s) = Vtex;
            pixels.emplace_back(x, y);
          }
        }
      }
    }

    if (_useSmudge) {
      _smudge.smudge(canvas, boundMin, spineSpline, length, thicknessMap);
    }

    for (const auto& p : pixels) {
      auto& vBuffer = canvas.getPaintLayer().getV_buffer();
      const auto x  = p[0U];
      const auto y  = p[1U];
      if ((x < 0) || (y < 0) || (x >= vBuffer.cols) || (y >= vBuffer.rows)) {
        continue;
      }
      const auto Vtex = thicknessMap(
        clamp(0, y - static_cast<int32_t>(boundMin[1U]), thicknessMap.rows - 1),
        clamp(0, x - static_cast<int32_t>(boundMin[0U]),
              thicknessMap.cols - 1));
      const auto Vcan = vBuffer(y, x);

      const auto Vsum = Vcan + Vtex;
      if (Vsum > static_cast<T>(0.0)) {
        const T sc = static_cast<T>(1.0) / Vsum;

        auto& K = canvas.getPaintLayer().getK_buffer()(y, x);
        auto& S = canvas.getPaintLayer().getS_buffer()(y, x);
        auto& V = vBuffer(y, x);

        K = (Vcan * K + Vtex * _paintStored[0U]) * sc;
        S = (Vcan * S + Vtex * _paintStored[1U]) * sc;
        V = std::max(Vtex, Vcan);
      }
    }
  }

  void enableSmudge(const bool enable) {
    _useSmudge = enable;
  }

 private:
  /**
   * @brief Brush stroke texture sample that can be warped along a trajectory or list of vertices.
   *
   */
  BrushStrokeSample _brushStrokeSample;

  /**
   * @brief The current paint the brush stores.
   *
   */
  std::array<vector_type, 2UL> _paintStored;

  /**
   * @brief
   *
   */
  double _radius = 0.0;

  /**
   * @brief
   *
   */
  Smudge<vector_type> _smudge;

  bool _useSmudge = true;
};
}  // namespace painty
