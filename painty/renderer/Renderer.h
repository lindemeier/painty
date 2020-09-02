/**
 * @file Renderer.h
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-05-15
 *
 */
#pragma once

#include "painty/image/Mat.h"
#include "painty/renderer/Canvas.h"
#include "painty/renderer/PaintLayer.h"

namespace painty {
template <class vector_type>
class Renderer final {
  using T                 = typename DataType<vector_type>::channel_type;
  static constexpr auto N = DataType<vector_type>::dim;

 public:
  /**
   * @brief Compose wet layer onto substrate.
   *
   * @return Mat<vector_type>
   */
  Mat<vector_type> compose(const PaintLayer<vector_type>& paintLayer,
                           const Mat<vector_type>& R0_buffer) const {
    Mat<vector_type> R1(R0_buffer.rows, R0_buffer.cols);

    auto& r1_data       = R1;
    const auto& r0_data = R0_buffer;

    const auto& K = paintLayer.getK_buffer();
    const auto& S = paintLayer.getS_buffer();
    const auto& V = paintLayer.getV_buffer();

    for (auto i = 0; i < static_cast<int32_t>(r0_data.total()); i++) {
      r1_data(i) = ComputeReflectance(K(i), S(i), r0_data(i), V(i));
    }
    return R1;
  }

  /**
   * @brief Compose current wet layer of canvas onto substrate.
   *
   * @return Mat<vector_type>
   */
  Mat<vector_type> compose(const Canvas<vector_type>& canvas) const {
    const auto& R0_buffer  = canvas.getR0();
    const auto& paintLayer = canvas.getPaintLayer();

    return compose(paintLayer, R0_buffer);
  }

  /**
   * @brief Render the canvas with directional light
   *
   * @return Mat<vector_type>
   */
  Mat<vector_type> render(const Canvas<vector_type>& canvas) const {
    const auto G = [](T NdotH, T NdotV, T VdotH, T NdotL) {
      T G1 = 2.0 * NdotH * NdotV / VdotH;
      T G2 = 2.0 * NdotH * NdotL / VdotH;
      return std::min(1.0, std::min(G1, G2));
    };

    const auto R_F = [](T VdotH, const vector_type& Ks) {
      return Ks + (vector_type::Ones() - Ks) * std::pow(1.0 - VdotH, 5.0);
    };

    const auto Beckmann = [](T NdotH, T m) {
      T A =
        1.0 / (std::pow(m, 2.0) + std::pow(NdotH, 4.0) * painty::Pi<double>);
      T B =
        std::exp(-std::pow(std::tan(std::acos(NdotH)), 2.0) / std::pow(m, 2.0));
      return A * B;
    };

    const auto compR = compose(canvas);

    const vector_type lightPos = {-200, -1500, -2000.};

    const vec2 size   = {2.0, 0.0};
    const auto width  = compR.cols;
    const auto height = compR.rows;

    const vec3 eyePos = {width / 2.0, height / 2.0, -100.};

    vector_type lightPower;
    lightPower.fill(15.0);
    vector_type Ks;
    Ks.fill(1.);      // surface specular color: equal to R_F(0)
    const T m = 0.5;  // material roughness (average slope of microfacets)
    const T s =
      0.2;  // percentage of incoming light which is specularly reflected

    Mat<T> heightMap = canvas.getPaintLayer().getV_buffer();

    Mat<vector_type> rgb(height, width);

    for (auto i = 0; i < height; ++i) {
      for (auto j = 0; j < width; ++j) {
        // compute normal
        const T s11 = heightMap(i, j);
        const T s01 =
          Interpolate(heightMap, {static_cast<T>(j) - 1.0, static_cast<T>(i)});
        const T s21 =
          Interpolate(heightMap, {static_cast<T>(j) + 1.0, static_cast<T>(i)});
        const T s10 =
          Interpolate(heightMap, {static_cast<T>(j), static_cast<T>(i) - 1.0});
        const T s12 =
          Interpolate(heightMap, {static_cast<T>(j), static_cast<T>(i) + 1.0});
        vector_type va = {size[0], size[1], s21 - s01};
        va             = va.normalized();
        vector_type vb = {size[1], size[0], s12 - s10};
        vb             = vb.normalized();
        // cross product
        vector_type n = va.cross(vb).normalized();
        n[2] *= -1.;

        const vec3 pixPos = {static_cast<T>(j), static_cast<T>(i), s11};
        // const vec3d lightDirection = vec3d(-0.3, -0.1, -0.3).normalized();
        const vec3 lightDirection = (lightPos - pixPos).normalized();
        const vec3 l              = (lightDirection).normalized();
        const vec3 v              = (eyePos - pixPos).normalized();
        const vec3 h              = (v + l).normalized();

        const vector_type Kd      = compR(i, j);  // surface diffuse color
        const vector_type ambient = Kd * 0.2;

        const T NdotH = std::max(0.0, n.dot(h));
        const T VdotH = std::max(0.0, v.dot(h));
        const T NdotV = std::max(0.0, n.dot(v));
        const T NdotL = std::max(0.0, n.dot(l));

        vector_type specular = vector_type::Zero();
        if (NdotL > 0.0 && NdotV > 0.0) {
          specular = (Beckmann(NdotH, m) * G(NdotH, NdotV, VdotH, NdotL) *
                      R_F(VdotH, Ks)) /
                     (NdotL * NdotV);
        }
        const vector_type beta =
          lightPower *
          (1.0 / (4.0 * Pi<double> * std::pow(lightDirection.norm(), 2.0)));
        const vector_type result =
          (beta * NdotL).array() * ((1.0 - s) * Kd + s * specular).array() +
          ambient.array() * Kd.array();

        for (auto u = 0U; u < N; u++) {
          rgb(i, j)[u] = std::min(std::max(result[u], 0.0), 1.0);
        }
      }
    }

    return rgb;
  }
};
}  // namespace painty
