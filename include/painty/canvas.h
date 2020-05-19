/**
 * @file canvas.h
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-05-15
 *
 */
#ifndef PAINTY_CANVAS_H
#define PAINTY_CANVAS_H

#include <algorithm>
#include <chrono>
#include <type_traits>

#include <painty/mat.h>
#include <painty/kubelka_munk.h>
#include <painty/paint_layer.h>
#include <painty/vec.h>

namespace painty
{
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
class Canvas final
{
public:
  Canvas(const uint32_t rows, const uint32_t cols)
    : _paintLayer(rows, cols)
    , _backgroundColor()
    , _R0_buffer(rows, cols)
    , _h_buffer(rows, cols)
    , _timeMap(rows, cols)
    , _dryingTime(60 * 1000)
  {
    _backgroundColor.fill(static_cast<T>(1.0));
    clear();
  }

  void clear()
  {
    _paintLayer.clear();

    const auto rows = _paintLayer.getRows();
    const auto cols = _paintLayer.getCols();
    _R0_buffer = Mat<vec<T, N>>(rows, cols);
    _h_buffer = Mat<T>(rows, cols);

    _timeMap = Mat<std::chrono::system_clock::time_point>(rows, cols);
    auto now = std::chrono::system_clock::now();
    for (auto& t : _timeMap.getData())
    {
      t = now;
    }

    auto& r0 = _R0_buffer.getData();
    auto& h = _h_buffer.getData();
    for (size_t i = 0U; i < r0.size(); i++)
    {
      r0[i] = _backgroundColor;
      h[i] = static_cast<T>(0.0);
    }
  }

  const Mat<vec<T, N>>& getR0() const
  {
    return _R0_buffer;
  }

  const Mat<T>& get_h() const
  {
    return _h_buffer;
  }

  Mat<vec<T, N>>& getR0()
  {
    return _R0_buffer;
  }

  Mat<T>& get_h()
  {
    return _h_buffer;
  }

  /**
   * @brief Compose current wet layer onto substrate.
   *
   * @return Mat<vec<T, N>>
   */
  Mat<vec<T, N>> composed() const
  {
    Mat<vec<T, N>> R1(_R0_buffer.getRows(), _R0_buffer.getCols());
    auto& r1_data = R1.getData();
    const auto& r0_data = _R0_buffer.getData();
    const auto& K = _paintLayer.getK_buffer().getData();
    const auto& S = _paintLayer.getS_buffer().getData();
    const auto& V = _paintLayer.getV_buffer().getData();
    for (size_t i = 0U; i < r0_data.size(); i++)
    {
      r1_data[i] = ComputeReflectance(K[i], S[i], r0_data[i], V[i]);
    }
    return R1;
  }

  Mat<vec<T, N>> getReflectanceLayerDry() const
  {
    return _R0_buffer.clone();
  }

  void setBackground(const Mat<vec<T, N>>& background)
  {
    clear();
    auto& r0_data = _R0_buffer.getData();
    const auto& b_data = background.getData();
    for (size_t i = 0; i < r0_data.size(); i++)
    {
      r0_data[i] = b_data[i];
    }
  }

  const PaintLayer<T, N>& getPaintLayer() const
  {
    return _paintLayer;
  }

  PaintLayer<T, N>& getPaintLayer()
  {
    return _paintLayer;
  }

  const Mat<std::chrono::system_clock::time_point>& getTimeMap() const
  {
    return _timeMap;
  }

  Mat<std::chrono::system_clock::time_point>& getTimeMap()
  {
    return _timeMap;
  }

  void checkDry(uint32_t x, uint32_t y, const std::chrono::system_clock::time_point& timePoint)
  {
    T v = _paintLayer.getV_buffer()(y, x);
    if ((_dryingTime.count() > 0.0) && (v > 0.0))
    {
      auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint - _timeMap(y, x));

      if (dur >= _dryingTime)
      {
        _h_buffer(y, x) += v;
        getR0()(y, x) =
            ComputeReflectance(_paintLayer.getK_buffer()(y, x), _paintLayer.getS_buffer()(y, x), getR0()(y, x), v);
        _paintLayer.getV_buffer()(y, x) = 0.0;
        _paintLayer.getK_buffer()(y, x).fill(0.0);
        _paintLayer.getS_buffer()(y, x).fill(0.0);
      }
      else
      {
        const T rate = dur.count() / static_cast<T>(_dryingTime.count());

        if (rate > 0.01)
        {
          // only dry a portion
          T vl = rate * v;  // amount of paint getting dry

          _h_buffer(y, x) += vl;
          T vr = v - vl;  // amount of paint left (being wet).
          getR0()(y, x) =
              ComputeReflectance(_paintLayer.getK_buffer()(y, x), _paintLayer.getS_buffer()(y, x), getR0()(y, x), vl);
          _paintLayer.getV_buffer()(y, x) = vr;
        }
      }
    }
    _timeMap(y, x) = timePoint;
  }

  void setDryingTime(std::chrono::milliseconds msecs)
  {
    _dryingTime = msecs;
  }

  /**
   * @brief Render the canvas with directional light
   *
   * @return Mat<vec<T, N>>
   */
  Mat<vec<T, N>> getLightedRendering() const
  {
    const auto G = [](T NdotH, T NdotV, T VdotH, T NdotL) {
      T G1 = 2.0 * NdotH * NdotV / VdotH;
      T G2 = 2.0 * NdotH * NdotL / VdotH;
      return std::min(1.0, std::min(G1, G2));
    };

    const auto R_F = [](T VdotH, const vec<T, N>& Ks) {
      vec<T, N> One;
      One.fill(1.0);
      return Ks + (One - Ks) * std::pow(1.0 - VdotH, 5.0);
    };

    const auto Beckmann = [](T NdotH, T m) {
      T A = 1.0 / (std::pow(m, 2.0) + std::pow(NdotH, 4.0) * painty::PI);
      T B = std::exp(-std::pow(std::tan(std::acos(NdotH)), 2.0) / std::pow(m, 2.0));
      return A * B;
    };

    const vec<T, N> lightPos = { -200, -1500, -2000. };

    const vec2 size = { 2.0, 0.0 };
    const auto width = _R0_buffer.getCols();
    const auto height = _R0_buffer.getRows();

    const vec3 eyePos = { width / 2.0, height / 2.0, -100. };

    vec<T, N> lightPower;
    lightPower.fill(15.0);
    vec<T, N> Ks;
    Ks.fill(1.);      // surface specular color: equal to R_F(0)
    const T m = 0.5;  // material roughness (average slope of microfacets)
    const T s = 0.2;  // percentage of incoming light which is specularly reflected

    Mat<T> heightMap = getPaintLayer().getV_buffer();
    auto compR = composed();

    Mat<vec<T, N>> rgb(compR.getRows(), compR.getCols());

    for (auto i = 0U; i < height; ++i)
    {
      for (auto j = 0U; j < width; ++j)
      {
        // compute normal
        const T s11 = heightMap(i, j);
        const T s01 = heightMap({ static_cast<T>(i), static_cast<T>(j) - 1.0 });
        const T s21 = heightMap({ static_cast<T>(i), static_cast<T>(j) + 1.0 });
        const T s10 = heightMap({ static_cast<T>(i) - 1.0, static_cast<T>(j) });
        const T s12 = heightMap({ static_cast<T>(i) + 1.0, static_cast<T>(j) });
        vec<T, N> va = { size[0], size[1], s21 - s01 };
        va = normalized(va);
        vec<T, N> vb = { size[1], size[0], s12 - s10 };
        va = normalized(vb);
        // cross product
        vec<T, N> n{ va[1] * vb[2] - va[2] * vb[1], va[2] * vb[0] - va[0] * vb[2], va[0] * vb[1] - va[1] * vb[0] };
        n[2] *= -1.;

        const vec3 pixPos = { static_cast<T>(j), static_cast<T>(i), s11 };
        // const vec3d lightDirection = vec3d(-0.3, -0.1, -0.3).normalized();
        const vec3 lightDirection = normalized(lightPos - pixPos);
        const vec3 l = normalized(lightDirection);
        const vec3 v = normalized(eyePos - pixPos);
        const vec3 h = normalized(v + l);

        const vec<T, N> Kd = compR(i, j);  // surface diffuse color
        const vec<T, N> ambient = Kd * 0.2;

        const T NdotH = std::max(0.0, dot(n, h));
        const T VdotH = std::max(0.0, dot(v, h));
        const T NdotV = std::max(0.0, dot(n, v));
        const T NdotL = std::max(0.0, dot(n, l));

        vec<T, N> specular;
        specular.fill(0.0);
        if (NdotL > 0.0 && NdotV > 0.0)
        {
          specular = (Beckmann(NdotH, m) * G(NdotH, NdotV, VdotH, NdotL) * R_F(VdotH, Ks)) / (NdotL * NdotV);
        }
        const vec<T, N> beta = lightPower * (1.0 / (4.0 * PI * std::pow(norm(lightDirection), 2.0)));
        const vec<T, N> result = (beta * NdotL) * ((1.0 - s) * Kd + s * specular) + ambient * Kd;
        
        for (auto i = 0U; i < N; i++) {
          rgb(i, j)[i] = std::min(std::max(result[i], 0.0), 1.0);
        }
      }
    }

    return rgb;
  }

private:
  /**
   * @brief Wet paint layer.
   *
   */
  PaintLayer<T, N> _paintLayer;
  /**
   * @brief Background color as init for substrate.
   *
   */
  vec<T, N> _backgroundColor;
  /**
   * @brief Substrate reflectance. Layer of dried paint.
   *
   */
  Mat<vec<T, N>> _R0_buffer;
  /**
   * @brief Height map.
   *
   */
  Mat<T> _h_buffer;

  /**
   * @brief Time pased since a cell was affected by paint.
   *
   */
  Mat<std::chrono::system_clock::time_point> _timeMap;

  /**
   * @brief Total time of drying process.
   *
   */
  std::chrono::milliseconds _dryingTime;
};
}  // namespace painty

#endif  // PAINTY_CANVAS_H
