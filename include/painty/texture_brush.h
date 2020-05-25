/**
 * @file texture_brush.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-19
 *
 */
#ifndef PAINTY_TEXTURE_BRUSH_H
#define PAINTY_TEXTURE_BRUSH_H

#include <painty/canvas.h>
#include <painty/brush_stroke_sample.h>
#include <painty/spline.h>

namespace painty
{
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
class TextureBrush final
{
private:
  using KS = std::array<vec<T, N>, 2UL>;

public:
  TextureBrush(const std::string& sampleDir) : _brushStrokeSample(sampleDir)
  {
    for (auto& c : _paintStored)
    {
      c.fill(static_cast<T>(0.1));
    }
  }

  /**
   * @brief Dip the brush in paint paint.
   *
   * @param paint
   */
  void dip(const KS& paint)
  {
    _paintStored = paint;
  }

  void applyTo(const std::vector<vec2>& vertices, Canvas<T, N>& canvas) const
  {
    if (vertices.empty())
    {
      return;
    }

    // compute bounding rectangle
    const auto radius = _brushStrokeSample.getWidth() / 2.0;

    auto boundMin = vertices.front();
    auto boundMax = vertices.front();
    for (const auto& xy : vertices)
    {
      boundMin[0U] = std::min(boundMin[0U], xy[0U]);
      boundMin[1U] = std::min(boundMin[1U], xy[1U]);
      boundMax[0U] = std::max(boundMax[0U], xy[0U]);
      boundMax[1U] = std::max(boundMax[1U], xy[1U]);
    }
    boundMin[0U] = std::max(boundMin[0U] - radius, 0.0);
    boundMax[0U] = std::min(boundMax[0U] + radius, static_cast<T>(canvas.getPaintLayer().getCols() - 1U));
    boundMin[1U] = std::max(boundMin[1U] - radius, 0.0);
    boundMax[1U] = std::min(boundMax[1U] + radius, static_cast<T>(canvas.getPaintLayer().getRows() - 1U));

    // spine
    SplineEval<std::vector<vec2>::const_iterator> spineSpline(vertices.cbegin(), vertices.cend());

    // construct frames around the vertices
    std::vector<vec2> upCanvas;
    upCanvas.reserve(vertices.size());
    std::vector<vec2> downCanvas;
    downCanvas.reserve(vertices.size());

    // frames around the texture
    std::vector<vec2> upTex;
    upTex.reserve(vertices.size());
    std::vector<vec2> downTex;
    downTex.reserve(vertices.size());

    constexpr auto ScaleGBC = 1.0;

    for (auto i = 0U; i < vertices.size(); ++i)
    {
      T u = static_cast<T>(i) / static_cast<T>(vertices.size() - 1);
      const auto c = spineSpline.cubic(u);

      // spine tangent vector
      auto t = spineSpline.cubicDerivative(u, 1).normalized();

      // compute perpendicular vector to spine
      const vec2 d = { -t[1], t[0] };

      const auto l = c - radius * d * ScaleGBC;
      const auto r = c + radius * d * ScaleGBC;

      upCanvas.push_back(l);
      downCanvas.push_back(r);

      upTex.push_back({ u, -ScaleGBC });
      downTex.push_back({ u, ScaleGBC });
    }
    upCanvas.insert(upCanvas.begin(), downCanvas.rbegin(), downCanvas.rend());

    upTex.insert(upTex.begin(), downTex.rbegin(), downTex.rend());

    // canvas coordinates to uv coordinates
    TextureWarp tex2uv;
    tex2uv.init(upCanvas, upTex);

    auto now = std::chrono::system_clock::now();

    std::vector<vec<uint32_t, 2U>> pixels;
    for (auto x = static_cast<uint32_t>(boundMin[0U]); x <= static_cast<uint32_t>(boundMax[0U]); x++)
    {
      for (auto y = static_cast<uint32_t>(boundMin[1U]); y <= static_cast<uint32_t>(boundMax[1U]); y++)
      {
        if ((x < 0) || (x >= canvas.getPaintLayer().getCols()) || (y < 0) || (y >= canvas.getPaintLayer().getRows()))
        {
          continue;
        }

        // if (!PointInPolyon(upCanvas, { static_cast<T>(x), static_cast<T>(y) }))
        // {
        //   continue;
        // }

        // transform canvas coordinates to uv local coordinates
        vec2 canvasUV = tex2uv.warp({ static_cast<T>(x), static_cast<T>(y) });

        // uv not in stroke
        if ((canvasUV[0U] < 0.0) || (canvasUV[0U] > 1.0) || (canvasUV[1U] < -1.0) || (canvasUV[1U] > 1.0))
        {
          continue;
        }

        // retrieve the height of the sample at uv
        const auto Vtex = _brushStrokeSample.getSampleAtUV(canvasUV);
        if (Vtex > 0.0)
        {
          canvas.checkDry(x, y, now);

          const auto Vcan = canvas.getPaintLayer().getV_buffer()(y, x);

          const auto Vsum = Vcan + Vtex;
          if (Vsum > static_cast<T>(0.0))
          {
            const T sc = static_cast<T>(1.0) / Vsum;

            auto& K = canvas.getPaintLayer().getK_buffer()(y, x);
            auto& S = canvas.getPaintLayer().getS_buffer()(y, x);
            auto& V = canvas.getPaintLayer().getV_buffer()(y, x);

            K = (Vcan * K + Vtex * _paintStored[0U]) * sc;
            S = (Vcan * S + Vtex * _paintStored[1U]) * sc;
            V = std::max(Vtex, Vcan);
          }
        }
      }
    }
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
  KS _paintStored;
};
}  // namespace painty

#endif  // PAINTY_TEXTURE_BRUSH_H
