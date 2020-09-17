/**
 * @file ColorConverter.hxx
 * @author Thomas Lindemeier
 * @brief Collection of color conversion functions.
 * @date 2019-04-14
 *
 */

#pragma once

#include <array>
#include <cmath>
#include <limits>

#include "painty/core/Math.hxx"
#include "painty/core/Vec.hxx"

namespace painty {

/**
 * @brief Class for color conversions with reference to a given
 * white point (illuminant).
 *
 * Only data types of the template form vec<Type, size_t> are supported. Only
 * floating points and data types with 3 components are supported.
 *
 * Most of the conversion and constants were taken from
 * http://brucelindbloom.com/index.html (last accessed 2019 April 14th)
 * Credits to that guy!
 */
template <class Scalar>
class ColorConverter {
  static constexpr auto N = 3;

  static constexpr auto Epsilon =
    std::numeric_limits<Scalar>::epsilon() * static_cast<Scalar>(1000.0);

 public:
  static constexpr std::array<Scalar, 3U> IM_ILLUMINANT_A   = {1.09850, 1.00000,
                                                             0.35585};
  static constexpr std::array<Scalar, 3U> IM_ILLUMINANT_B   = {0.99072, 1.00000,
                                                             0.85223};
  static constexpr std::array<Scalar, 3U> IM_ILLUMINANT_C   = {0.98074, 1.00000,
                                                             1.18232};
  static constexpr std::array<Scalar, 3U> IM_ILLUMINANT_D50 = {0.96422, 1.00000,
                                                               0.82521};
  static constexpr std::array<Scalar, 3U> IM_ILLUMINANT_D55 = {0.95682, 1.00000,
                                                               0.92149};
  static constexpr std::array<Scalar, 3U> IM_ILLUMINANT_D65 = {0.95047, 1.00000,
                                                               1.08883};
  static constexpr std::array<Scalar, 3U> IM_ILLUMINANT_D75 = {0.94972, 1.00000,
                                                               1.22638};
  static constexpr std::array<Scalar, 3U> IM_ILLUMINANT_E   = {1.00000, 1.00000,
                                                             1.00000};
  static constexpr std::array<Scalar, 3U> IM_ILLUMINANT_2   = {0.99186, 1.00000,
                                                             0.67393};
  static constexpr std::array<Scalar, 3U> IM_ILLUMINANT_7   = {0.95041, 1.00000,
                                                             1.08747};
  static constexpr std::array<Scalar, 3U> IM_ILLUMINANT_11  = {1.00962, 1.00000,
                                                              0.64350};

 private:  // private constants
  // sRGB D65, http://brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
  static constexpr double XYZ2RGB_MATRIX[3U][3U] = {
    {3.2404542, -1.5371385, -0.4985314},
    {-0.9692660, 1.8760108, 0.0415560},
    {0.0556434, -0.2040259, 1.0572252}};

  // sRGB D65, http://brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
  static constexpr double RGB2XYZ_MATRIX[3U][3U] = {
    {0.4124564, 0.3575761, 0.1804375},
    {0.2126729, 0.7151522, 0.0721750},
    {0.0193339, 0.1191920, 0.9503041}};

 public:
  ColorConverter(const std::array<Scalar, 3U>& whitepoint = IM_ILLUMINANT_D65)
      : illuminant(whitepoint) {}

  void lab2xyz(const vec<Scalar, N>& Lab, vec<Scalar, N>& XYZ) const {
    // chromatic adaption, reference white
    XYZ[1] = illuminant[1] * fi((1. / 116.) * (Lab[0] + 16.));  // Y
    XYZ[0] = illuminant[0] *
             fi((1. / 116.) * (Lab[0] + 16.) + (1. / 500.) * Lab[1]);  // X
    XYZ[2] = illuminant[2] *
             fi((1. / 116.) * (Lab[0] + 16.) - (1. / 200.) * Lab[2]);  // Z
  }

  void xyz2lab(const vec<Scalar, N>& XYZ, vec<Scalar, N>& Lab) const {
    Lab[0] = 116. * f(XYZ[1] / illuminant[1]) - 16.;
    Lab[1] = 500. * (f(XYZ[0] / illuminant[0]) - f(XYZ[1] / illuminant[1]));
    Lab[2] = 200. * (f(XYZ[1] / illuminant[1]) - f(XYZ[2] / illuminant[2]));
  }

  void lab2LCHab(const vec<Scalar, N>& Lab, vec<Scalar, N>& LCHab) const {
    LCHab[0] = Lab[0];                                        // [0,100]
    LCHab[1] = std::sqrt(Lab[1] * Lab[1] + Lab[2] * Lab[2]);  // [0,100]

    LCHab[2] = std::atan2(Lab[2], Lab[1]);
    if (LCHab[2] < 0) {
      LCHab[2] += Pi<Scalar> * 2.;  // [0, 2pi]
    }
  }

  void LCHab2lab(const vec<Scalar, N>& LCHab, vec<Scalar, N>& Lab) const {
    Lab[0]   = LCHab[0];
    Scalar h = LCHab[2];
    if (h > Pi<Scalar>) {
      h -= Pi<Scalar> * 2.;  // [0, 2pi]
    }
    Lab[1] = LCHab[1] * std::cos(h);
    Lab[2] = LCHab[1] * std::sin(h);
  }

  // http://en.wikipedia.org/wiki/RYB_color_model
  // http://threekings.tk/mirror/ryb_TR.pdf
  void ryb2rgb(const vec<Scalar, N>& ryb, vec<Scalar, N>& rgb) const {
    auto cubicInt = [](Scalar t, Scalar A, Scalar B) {
      Scalar weight = t * t * (3 - 2 * t);
      return A + weight * (B - A);
    };
    Scalar x0;
    Scalar x1;
    Scalar x2;
    Scalar x3;
    Scalar y0;
    Scalar y1;
    // red
    x0     = cubicInt(ryb[2], 1., 0.163);
    x1     = cubicInt(ryb[2], 1., 0.);
    x2     = cubicInt(ryb[2], 1., 0.5);
    x3     = cubicInt(ryb[2], 1., 0.2);
    y0     = cubicInt(ryb[1], x0, x1);
    y1     = cubicInt(ryb[1], x2, x3);
    rgb[0] = cubicInt(ryb[0], y0, y1);
    // green
    x0     = cubicInt(ryb[2], 1., 0.373);
    x1     = cubicInt(ryb[2], 1., 0.66);
    x2     = cubicInt(ryb[2], 0., 0.);
    x3     = cubicInt(ryb[2], 0.5, 0.094);
    y0     = cubicInt(ryb[1], x0, x1);
    y1     = cubicInt(ryb[1], x2, x3);
    rgb[1] = cubicInt(ryb[0], y0, y1);
    // blue
    x0     = cubicInt(ryb[2], 1., 0.6);
    x1     = cubicInt(ryb[2], 0., 0.2);
    x2     = cubicInt(ryb[2], 0., 0.5);
    x3     = cubicInt(ryb[2], 0., 0.);
    y0     = cubicInt(ryb[1], x0, x1);
    y1     = cubicInt(ryb[1], x2, x3);
    rgb[2] = cubicInt(ryb[0], y0, y1);
  }

  // rgb to cmy
  void rgb2cmy(const vec<Scalar, N>& rgb, vec<Scalar, N>& cmy) const {
    cmy[0] = 1. - rgb[0];
    cmy[1] = 1. - rgb[1];
    cmy[2] = 1. - rgb[2];
  }

  // cmy to rgb
  void cmy2rgb(const vec<Scalar, N>& cmy, vec<Scalar, N>& rgb) const {
    rgb[0] = 1. - cmy[0];
    rgb[1] = 1. - cmy[1];
    rgb[2] = 1. - cmy[2];
  }

  // uses sRGB chromatic adapted matrix
  void rgb2xyz(const vec<Scalar, N>& rgb, vec<Scalar, N>& XYZ) const {
    XYZ[0] = RGB2XYZ_MATRIX[0][0] * rgb[0] + RGB2XYZ_MATRIX[0][1] * rgb[1] +
             RGB2XYZ_MATRIX[0][2] * rgb[2];
    XYZ[1] = RGB2XYZ_MATRIX[1][0] * rgb[0] + RGB2XYZ_MATRIX[1][1] * rgb[1] +
             RGB2XYZ_MATRIX[1][2] * rgb[2];
    XYZ[2] = RGB2XYZ_MATRIX[2][0] * rgb[0] + RGB2XYZ_MATRIX[2][1] * rgb[1] +
             RGB2XYZ_MATRIX[2][2] * rgb[2];
  }

  // uses sRGB chromatic adapted matrix
  void xyz2rgb(const vec<Scalar, N>& XYZ, vec<Scalar, N>& rgb) const {
    rgb[0] = XYZ2RGB_MATRIX[0][0] * XYZ[0] + XYZ2RGB_MATRIX[0][1] * XYZ[1] +
             XYZ2RGB_MATRIX[0][2] * XYZ[2];
    rgb[1] = XYZ2RGB_MATRIX[1][0] * XYZ[0] + XYZ2RGB_MATRIX[1][1] * XYZ[1] +
             XYZ2RGB_MATRIX[1][2] * XYZ[2];
    rgb[2] = XYZ2RGB_MATRIX[2][0] * XYZ[0] + XYZ2RGB_MATRIX[2][1] * XYZ[1] +
             XYZ2RGB_MATRIX[2][2] * XYZ[2];
  }

  // make linear rgb, no chromatic adaption
  void srgb2rgb(const Scalar s, Scalar& l) const {
    if (s <= 0.0404482362771082) {
      l = s / 12.92;
    } else {
      l = std::pow(((s + 0.055) / 1.055), 2.4);
    }
  }

  // make sRGB, with gamma
  void rgb2srgb(const Scalar l, Scalar& s) const {
    if (l <= 0.00313066844250063) {
      s = l * 12.92;
    } else {
      s = 1.055 * std::pow(l, 1. / 2.4) - 0.055;
    }
  }

  // make linear rgb, no chromatic adaption
  void srgb2rgb(const vec<Scalar, N>& srgb, vec<Scalar, N>& rgb) const {
    for (auto i = 0U; i < 3U; i++) {
      srgb2rgb(srgb[i], rgb[i]);
    }
  }

  // make sRGB, with gamma
  void rgb2srgb(const vec<Scalar, N>& rgb, vec<Scalar, N>& srgb) const {
    for (auto i = 0U; i < 3U; ++i) {
      rgb2srgb(rgb[i], srgb[i]);
    }
  }

  // Lab (D50) -> XYZ -> rgb (D65) -> sRGB (D65)
  void lab2srgb(const vec<Scalar, N>& Lab, vec<Scalar, N>& srgb) const {
    vec<Scalar, N> XYZ;
    lab2xyz(Lab, XYZ);
    vec<Scalar, N> rgb;
    xyz2rgb(XYZ, rgb);
    rgb2srgb(rgb, srgb);
  }

  // sRGB (D65) -> rgb (D65) -> XYZ -> Lab
  void srgb2lab(const vec<Scalar, N>& srgb, vec<Scalar, N>& Lab) const {
    vec<Scalar, N> rgb;
    srgb2rgb(srgb, rgb);
    vec<Scalar, N> XYZ;
    rgb2xyz(rgb, XYZ);
    xyz2lab(XYZ, Lab);
  }

  // Lab  -> XYZ -> rgb (D65)
  void lab2rgb(const vec<Scalar, N>& Lab, vec<Scalar, N>& rgb) const {
    vec<Scalar, N> XYZ;
    lab2xyz(Lab, XYZ);
    xyz2rgb(XYZ, rgb);
  }

  // rgb (D65) -> XYZ -> Lab
  void rgb2lab(const vec<Scalar, N>& rgb, vec<Scalar, N>& Lab) const {
    vec<Scalar, N> XYZ;
    rgb2xyz(rgb, XYZ);
    xyz2lab(XYZ, Lab);
  }

  // XYZ -> rgb (D65) -> sRGB (D65)
  void xyz2srgb(const vec<Scalar, N>& XYZ, vec<Scalar, N>& srgb) const {
    vec<Scalar, N> rgb;
    xyz2rgb(XYZ, rgb);
    rgb2srgb(rgb, srgb);
  }

  // [0..1] -> [0..1]
  void hsv2srgb(const vec<Scalar, N>& hsv, vec<Scalar, N>& srgb) const {
    const Scalar h   = (360. * hsv[0]) / 60.;
    const int32_t hi = static_cast<int32_t>(std::floor(h));
    Scalar f         = (h - hi);

    Scalar p = hsv[2] * (1 - hsv[1]);
    Scalar q = hsv[2] * (1 - hsv[1] * f);
    Scalar t = hsv[2] * (1 - hsv[1] * (1 - f));

    if (hi == 1) {
      srgb[0] = q;
      srgb[1] = hsv[2];
      srgb[2] = p;
    } else if (hi == 2) {
      srgb[0] = p;
      srgb[1] = hsv[2];
      srgb[2] = t;
    } else if (hi == 3) {
      srgb[0] = p;
      srgb[1] = q;
      srgb[2] = hsv[2];
    } else if (hi == 4) {
      srgb[0] = t;
      srgb[1] = p;
      srgb[2] = hsv[2];
    } else if (hi == 5) {
      srgb[0] = hsv[2];
      srgb[1] = p;
      srgb[2] = q;
    } else {
      srgb[0] = hsv[2];
      srgb[1] = t;
      srgb[2] = p;
    }
  }

  // [0..1] -> [0..1]
  void srgb2hsv(const vec<Scalar, N>& srgb, vec<Scalar, N>& hsv) const {
    Scalar min;
    Scalar max;
    Scalar delMax;

    min    = std::min<Scalar>(std::min<Scalar>(srgb[0], srgb[1]), srgb[2]);
    max    = std::max<Scalar>(std::max<Scalar>(srgb[0], srgb[1]), srgb[2]);
    delMax = 1. / (max - min);

    const Scalar fa = 1. / 360.0;

    if (fuzzyCompare(max, min, Epsilon)) {
      hsv[0] = 0;
    } else if (fuzzyCompare(max, srgb[0], Epsilon)) {
      hsv[0] = 60.0 * (0 + (srgb[1] - srgb[2]) * delMax);
    } else if (fuzzyCompare(max, srgb[1], Epsilon)) {
      hsv[0] = 60.0 * (2 + (srgb[2] - srgb[0]) * delMax);
    } else if (fuzzyCompare(max, srgb[2], Epsilon)) {
      hsv[0] = 60.0 * (4 + (srgb[0] - srgb[1]) * delMax);
    }

    if (hsv[0] < 0.0) {
      hsv[0] += 360.0;
    }

    if (fuzzyCompare(max, 0.0, Epsilon)) {
      hsv[1] = 0.0;
    } else {
      hsv[1] = (max - min) / max;
    }
    hsv[2] = max;

    hsv[0] *= fa;
  }

  // sRGB (D65) -> XYZ
  void srgb2xyz(const vec<Scalar, N>& srgb, vec<Scalar, N>& XYZ) const {
    vec<Scalar, N> rgb;
    srgb2rgb(srgb, rgb);
    rgb2xyz(rgb, XYZ);
  }

  void xyz2xyY(const vec<Scalar, N>& XYZ, vec<Scalar, N>& xyY) const {
    xyY[0] = XYZ[0] / (XYZ[0] + XYZ[1] + XYZ[2]);
    xyY[1] = XYZ[1] / (XYZ[0] + XYZ[1] + XYZ[2]);
    xyY[2] = XYZ[1];
  }

  void xyY2xyz(const vec<Scalar, N>& xyY, vec<Scalar, N>& XYZ) const {
    XYZ[1] = xyY[2];
    XYZ[0] = (xyY[2] / xyY[1]) * xyY[0];
    XYZ[2] = (xyY[2] / xyY[1]) * (1 - xyY[0] - xyY[1]);
  }

  void Luv2XYZ(const vec<Scalar, N>& Luv, vec<Scalar, N>& XYZ) const {
    const Scalar eps  = 216. / 24389.;
    const Scalar k    = 24389. / 27.;
    const Scalar keps = k * eps;

    XYZ[1] =
      (Luv[0] > keps) ? (std::pow((Luv[0] + 16.) / 116., 3.)) : (Luv[1] / k);

    Scalar Xr;
    Scalar Yr;
    Scalar Zr;
    Xr = illuminant[0];
    Yr = illuminant[1];
    Zr = illuminant[2];

    Scalar u0;
    Scalar v0;
    u0 = (4. * Xr) / (Xr + 15. * Yr + 3. * Zr);
    v0 = (9. * Yr) / (Xr + 15. * Yr + 3. * Zr);

    Scalar a;
    Scalar b;
    Scalar c;
    Scalar d;
    a = (1. / 3.) * (((52. * Luv[0]) / (Luv[1] + 13. * Luv[0] * u0)) - 1.);
    b = -5. * XYZ[1];
    c = -(1. / 3.);
    d = XYZ[1] * (((39. * Luv[0]) / (Luv[2] + 13. * Luv[0] * v0)) - 5.);

    XYZ[0] = (d - b) / (a - c);
    XYZ[2] = XYZ[0] * a + b;
  }

  void Yuv2rgb(const vec<Scalar, N>& Yuv, vec<Scalar, N>& rgb) const {
    rgb[2] = 1.164 * (Yuv[0] - 16) + 2.018 * (Yuv[1] - 128.);
    rgb[1] =
      1.164 * (Yuv[0] - 16) - 0.813 * (Yuv[2] - 128.) - 0.391 * (Yuv[1] - 128.);
    rgb[0] = 1.164 * (Yuv[0] - 16) + 1.596 * (Yuv[2] - 128.);

    const Scalar s = (1. / 255.);
    rgb[0] *= s;
    rgb[1] *= s;
    rgb[2] *= s;
  }

  void rgb2Yuv(const vec<Scalar, N>& rgb, vec<Scalar, N>& Yuv) const {
    vec<Scalar, N> rgb_scaled;
    rgb_scaled[0] = rgb[0] * 255.;
    rgb_scaled[1] = rgb[1] * 255.;
    rgb_scaled[2] = rgb[2] * 255.;
    Yuv[0]        = (0.257 * rgb_scaled[0]) + (0.504 * rgb_scaled[1]) +
             (0.098 * rgb_scaled[2]) + 16;
    Yuv[2] = (0.439 * rgb_scaled[0]) - (0.368 * rgb_scaled[1]) -
             (0.071 * rgb_scaled[2]) + 128;
    Yuv[1] = -(0.148 * rgb_scaled[0]) - (0.291 * rgb_scaled[1]) +
             (0.439 * rgb_scaled[2]) + 128;
  }

  void XYZ2Luv(const vec<Scalar, N>& XYZ, vec<Scalar, N>& Luv) const {
    const Scalar eps = 216. / 24389.;
    const Scalar k   = 24389. / 27.;

    // chromatic adaption, reference white
    Scalar Xr = illuminant[0];
    Scalar Yr = illuminant[1];
    Scalar Zr = illuminant[2];

    Scalar yr = XYZ[1] / Yr;

    Luv[0] = (yr > eps) ? (116. * std::pow(yr, 1. / 3.) - 16.) : k * yr;

    Scalar nen = XYZ[0] + 15. * XYZ[1] + 3 * XYZ[2];
    Scalar u_  = (4 * XYZ[0]) / (nen);
    Scalar v_  = (9 * XYZ[1]) / (nen);
    nen        = Xr + 15. * Yr + 3 * Zr;
    Scalar ur_ = (4 * Xr) / (nen);
    Scalar vr_ = (9 * Yr) / (nen);

    Luv[1] = 13. * Luv[0] * (u_ - ur_);
    Luv[2] = 13. * Luv[0] * (v_ - vr_);
  }

  void Luv2LCHuv(const vec<Scalar, N>& Luv, vec<Scalar, N>& LCHuv) const {
    LCHuv[0] = Luv[0];
    LCHuv[1] = std::sqrt((Luv[1] * Luv[1]) + (Luv[2] * Luv[2]));
    LCHuv[2] = atan2(Luv[2], Luv[1]);
  }

  void LCHuv2Luv(const vec<Scalar, N>& LCHuv, vec<Scalar, N>& Luv) const {
    Luv[0] = LCHuv[0];
    Luv[1] = LCHuv[1] * cos(LCHuv[2]);
    Luv[2] = LCHuv[1] * sin(LCHuv[2]);
  }

  void srgb2CIELCHab(const vec<Scalar, N>& srgb,
                     vec<Scalar, N>& CIELCHab) const {
    vec<Scalar, N> v;
    srgb2lab(srgb, v);
    lab2LCHab(v, CIELCHab);
  }

  void rgb2CIELCHab(const vec<Scalar, N>& rgb, vec<Scalar, N>& CIELCHab) const {
    vec<Scalar, N> v;
    rgb2lab(rgb, v);
    lab2LCHab(v, CIELCHab);
  }

  void CIELCHab2srgb(const vec<Scalar, N>& LCHab, vec<Scalar, N>& srgb) const {
    vec<Scalar, N> v;
    LCHab2lab(LCHab, v);
    lab2srgb(v, srgb);
  }

  void CIELCHab2rgb(const vec<Scalar, N>& CIELCHab, vec<Scalar, N>& rgb) const {
    vec<Scalar, N> v;
    LCHab2lab(CIELCHab, v);
    lab2rgb(v, rgb);
  }

  enum class Conversion {
    CIELab_2_XYZ,
    XYZ_2_CIELab,
    CIELab_2_LCHab,  // CIE LCHab
    LCH_ab_2_CIELab,
    ryb_2_rgb,  // http://en.wikipedia.org/wiki/RYB_color_model,
                // http://threekings.tk/mirror/ryb_TR.pdf
    rgb_2_cmy,
    cmy_2_rgb,
    rgb_2_XYZ,        // uses sRGB chromatic adapted matrix
    XYZ_2_rgb,        // uses sRGB chromatic adapted matrix
    srgb_2_rgb,       // make linear rgb, no chromatic adaption
    rgb_2_srgb,       // make sRGB, with gamma
    CIELab_2_srgb,    // Lab (whitepoint) -> XYZ -> rgb (D65) -> sRGB (D65)
    srgb_2_CIELab,    // sRGB (D65) -> rgb (D65) -> XYZ -> Lab (whitepoint)
    srgb_2_CIELCHab,  // sRGB (D65) -> rgb (D65) -> XYZ -> Lab (whitepoint)
    CIELCHab_2_srgb,  // sRGB (D65) -> rgb (D65) -> XYZ -> Lab (whitepoint)
    rgb_2_CIELCHab,
    CIELCHab_2_rgb,
    CIELab_2_rgb,  // Lab (whitepoint) -> XYZ -> rgb (D65)
    rgb_2_CIELab,  // rgb (D65) -> XYZ -> Lab (D50)
    XYZ_2_srgb,    // XYZ -> rgb (D65) -> sRGB (D65)
    hsv_2_srgb,    // [0..1] -> [0..1]
    srgb_2_hsv,    // [0..1] -> [0..1]
    srgb_2_XYZ,    // sRGB (D65) -> XYZ
    XYZ_2_xyY,
    xyY_2_XYZ,
    Luv_2_XYZ,
    XYZ_2_Luv,
    Luv_2_LCH_uv,  // CIE LCHuv
    LCH_uv_2_Luv,
    Yuv_2_rgb,  // Y Cb Cr
    rgb_2_Yuv
  };

  void convert(const vec<Scalar, N>& input, vec<Scalar, N>& output,
               Conversion conversion) const {
    switch (conversion) {
      case Conversion::CIELab_2_XYZ: {
        lab2xyz(input, output);
        break;
      }
      case Conversion::XYZ_2_CIELab: {
        xyz2lab(input, output);
        break;
      }
      case Conversion::CIELab_2_LCHab: {
        lab2LCHab(input, output);
        break;
      }
      case Conversion::LCH_ab_2_CIELab: {
        LCHab2lab(input, output);
        break;
      }
      case Conversion::ryb_2_rgb: {
        ryb2rgb(input, output);
        break;
      }
      case Conversion::rgb_2_cmy: {
        rgb2cmy(input, output);
        break;
      }
      case Conversion::cmy_2_rgb: {
        cmy2rgb(input, output);
        break;
      }
      case Conversion::rgb_2_XYZ: {
        rgb2xyz(input, output);
        break;
      }
      case Conversion::XYZ_2_rgb: {
        xyz2rgb(input, output);
        break;
      }
      case Conversion::srgb_2_rgb: {
        srgb2rgb(input, output);
        break;
      }
      case Conversion::rgb_2_srgb: {
        rgb2srgb(input, output);
        break;
      }
      case Conversion::CIELab_2_srgb: {
        lab2srgb(input, output);
        break;
      }
      case Conversion::srgb_2_CIELab: {
        srgb2lab(input, output);
        break;
      }
      case Conversion::CIELab_2_rgb: {
        lab2rgb(input, output);
        break;
      }
      case Conversion::rgb_2_CIELab: {
        rgb2lab(input, output);
        break;
      }
      case Conversion::XYZ_2_srgb: {
        xyz2srgb(input, output);
        break;
      }
      case Conversion::hsv_2_srgb: {
        hsv2srgb(input, output);
        break;
      }
      case Conversion::srgb_2_hsv: {
        srgb2hsv(input, output);
        break;
      }
      case Conversion::srgb_2_XYZ: {
        srgb2xyz(input, output);
        break;
      }
      case Conversion::XYZ_2_xyY: {
        xyz2xyY(input, output);
        break;
      }
      case Conversion::xyY_2_XYZ: {
        xyY2xyz(input, output);
        break;
      }
      case Conversion::Luv_2_XYZ: {
        Luv2XYZ(input, output);
        break;
      }
      case Conversion::XYZ_2_Luv: {
        XYZ2Luv(input, output);
        break;
      }
      case Conversion::Luv_2_LCH_uv: {
        Luv2LCHuv(input, output);
        break;
      }
      case Conversion::LCH_uv_2_Luv: {
        LCHuv2Luv(input, output);
        break;
      }
      case Conversion::Yuv_2_rgb: {
        Yuv2rgb(input, output);
        break;
      }
      case Conversion::rgb_2_Yuv: {
        rgb2Yuv(input, output);
        break;
      }
      case Conversion::srgb_2_CIELCHab: {
        vec<Scalar, N> v;
        srgb2lab(input, v);
        lab2LCHab(v, output);
        break;
      }
      case Conversion::rgb_2_CIELCHab: {
        vec<Scalar, N> v;
        rgb2lab(input, v);
        lab2LCHab(v, output);
        break;
      }
      case Conversion::CIELCHab_2_srgb: {
        vec<Scalar, N> v;
        LCHab2lab(input, v);
        lab2srgb(v, output);
        break;
      }
      case Conversion::CIELCHab_2_rgb: {
        vec<Scalar, N> v;
        LCHab2lab(input, v);
        lab2rgb(v, output);
        break;
      }
    }
  }

  /**
   * @brief Compute difference of two given colors.
   *
   * Ported to C++ from matlab script
   * (https://github.com/scienstanford/iqmetrics/blob/master/SCIELAB_FR/deltaE2000.m)
   *
   * Remarks:
   * Refer to https://en.wikipedia.org/wiki/Color_difference for details.
   *
   * @param lab1
   * @param lab2
   * @return Scalar
   */
  static Scalar ColorDifferenceCIEDE2000(const vec<Scalar, N>& lab1,
                                         const vec<Scalar, N>& lab2) {
    Scalar Lstd = lab1[0];
    Scalar astd = lab1[1];
    Scalar bstd = lab1[2];

    Scalar Lsample = lab2[0];
    Scalar asample = lab2[1];
    Scalar bsample = lab2[2];

    Scalar Cabstd    = std::sqrt(astd * astd + bstd * bstd);
    Scalar Cabsample = std::sqrt(asample * asample + bsample * bsample);

    Scalar Cabarithmean = (Cabstd + Cabsample) / 2.;

    Scalar G =
      0.5 * (1. - std::sqrt(std::pow(Cabarithmean, 7.) /
                            (std::pow(Cabarithmean, 7.) + std::pow(25., 7.))));

    Scalar apstd    = (1. + G) * astd;     // aprime in paper
    Scalar apsample = (1. + G) * asample;  // aprime in paper
    Scalar Cpsample = std::sqrt(apsample * apsample + bsample * bsample);

    Scalar Cpstd = std::sqrt(apstd * apstd + bstd * bstd);
    // Compute product of chromas
    Scalar Cpprod = (Cpsample * Cpstd);

    // Ensure hue is between 0 and 2pi
    Scalar hpstd = std::atan2(bstd, apstd);
    if (hpstd < 0) {
      hpstd += 2. * Pi<Scalar>;  // rollover ones that come -ve
    }

    Scalar hpsample = std::atan2(bsample, apsample);
    if (hpsample < 0) {
      hpsample += 2. * Pi<Scalar>;
    }
    if (fuzzyCompare((fabs(apsample) + fabs(bsample)), 0., Epsilon)) {
      hpsample = 0.;
    }

    Scalar dL = (Lsample - Lstd);
    Scalar dC = (Cpsample - Cpstd);

    // Computation of hue difference
    Scalar dhp = (hpsample - hpstd);
    if (dhp > Pi<Scalar>) {
      dhp -= 2. * Pi<Scalar>;
    }
    if (dhp < -Pi<Scalar>) {
      dhp += 2. * Pi<Scalar>;
    }
    // set chroma difference to zero if the product of chromas is zero
    if (fuzzyCompare(Cpprod, 0., Epsilon)) {
      dhp = 0.;
    }

    // Note that the defining equations actually need
    // signed Hue and chroma differences which is different
    // from prior color difference formulae

    Scalar dH = 2. * std::sqrt(Cpprod) * sin(dhp / 2.);
    //%dH2 = 4*Cpprod.*(sin(dhp/2)).^2;

    // weighting functions
    Scalar Lp = (Lsample + Lstd) / 2.;
    Scalar Cp = (Cpstd + Cpsample) / 2.;

    // Average Hue Computation
    // This is equivalent to that in the paper but simpler programmatically.
    // Note average hue is computed in radians and converted to degrees only
    // where needed
    Scalar hp = (hpstd + hpsample) / 2.;
    // Identify positions for which abs hue diff exceeds 180 degrees
    if (fabs(hpstd - hpsample) > Pi<Scalar>) {
      hp -= Pi<Scalar>;
    }
    // rollover ones that come -ve
    if (hp < 0) {
      hp += 2. * Pi<Scalar>;
    }

    // Check if one of the chroma values is zero, in which case set
    // mean hue to the sum which is equivalent to other value
    if (fuzzyCompare(Cpprod, 0., Epsilon)) {
      hp = hpsample + hpstd;
    }

    Scalar Lpm502 = (Lp - 50.) * (Lp - 50.);
    Scalar Sl     = 1. + 0.015 * Lpm502 / std::sqrt(20.0 + Lpm502);
    Scalar Sc     = 1. + 0.045 * Cp;
    Scalar Ta     = 1. - 0.17 * std::cos(hp - Pi<Scalar> / 6.) +
                0.24 * std::cos(2. * hp) +
                0.32 * std::cos(3. * hp + Pi<Scalar> / 30.) -
                0.20 * std::cos(4. * hp - 63. * Pi<Scalar> / 180.);
    Scalar Sh = 1. + 0.015 * Cp * Ta;
    Scalar delthetarad =
      (30. * Pi<Scalar> / 180.) *
      std::exp(-std::pow(((180. / Pi<Scalar> * hp - 275.) / 25.), 2.));
    Scalar Rc =
      2. * std::sqrt(std::pow(Cp, 7.) / (std::pow(Cp, 7.) + std::pow(25., 7.)));
    Scalar RT = -std::sin(2.0 * delthetarad) * Rc;

    // The CIE 00 color difference
    return std::sqrt(std::pow((dL / Sl), 2.) + std::pow((dC / Sc), 2.) +
                     std::pow((dH / Sh), 2.) + RT * (dC / Sc) * (dH / Sh));
  }

  /**
   * @brief Simplified color difference. Keeps values between 0 and 1 and clamps large differences.
   *
   * @param lab1
   * @param lab2
   * @return Scalar
   */
  static Scalar ColorDifference(const vec<Scalar, N>& lab1,
                                const vec<Scalar, N>& lab2) {
    static constexpr Scalar d0 = 100.;
    Scalar d                   = ColorDifferenceCIEDE2000(lab1, lab2);
    if (d >= 0.0 && d <= d0) {
      return d / d0;
    }
    return 1.0;
  }

 private:
  const std::array<Scalar, 3U> illuminant;

  static Scalar f(Scalar t) {
    return (t > std::pow<Scalar>(6. / 29., 3.))
             ? std::pow<Scalar>(t, 1. / 3.)
             : (1. / 3.) * std::pow<Scalar>(29. / 6., 2.) * t + (4. / 29.);
  }

  static Scalar fi(Scalar t) {
    return (t > 6. / 29.)
             ? std::pow<Scalar>(t, 3.)
             : 3. * std::pow<Scalar>(6. / 29., 2.) * (t - (4. / 29.));
  }
};

}  // namespace painty
