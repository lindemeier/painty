/**
 * @file brush_stroke_sample.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-01
 */
#include <painty/brush_stroke_sample.h>

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include <painty/brush_stroke_sample.h>
#include <painty/image_io.h>

painty::BrushStrokeSample::BrushStrokeSample()
{
  loadSample("/home/tsl/development/painty/data/sample_0");
}

void painty::BrushStrokeSample::addCorr_l(const vec2& texPos, const vec2& uv)
{
  _txy_l.push_back(texPos);
  _puv_l.push_back(uv);
}

void painty::BrushStrokeSample::addCorr_c(const vec2& texPos, const vec2& uv)
{
  _txy_c.push_back(texPos);
  _puv_c.push_back(uv);
}

void painty::BrushStrokeSample::addCorr_r(const vec2& texPos, const vec2& uv)
{
  _txy_r.push_back(texPos);
  _puv_r.push_back(uv);
}

const painty::Mat<double>& painty::BrushStrokeSample::getThicknessMap() const
{
  return _thickness_map;
}

void painty::BrushStrokeSample::setThicknessMap(const Mat<double>& thicknessMap)
{
  _thickness_map = thicknessMap;
}

/**
 * @brief Sample the brush stroke texture at xy.
 *
 * @param xy
 *
 * @return double 0.0 if xy outside of texture
 */
double painty::BrushStrokeSample::getSampleAt(const vec2& xy) const
{
  // outside of texture
  if (xy[0] < 0.0 || xy[1] < 0.0 || xy[0] >= _thickness_map.getCols() || xy[1] >= _thickness_map.getRows())
  {
    return 0.0;
  }

  // get texture value using bilinear interpolation
  return _thickness_map(xy);
}

/**
 * @brief Sample the brush stroke texture at a canvas coordinate uv. The uv is warped implicitly to stroke sample space.
 * Be sure to warp
 *
 * @param uv
 *
 * @return double 0.0 if xy outside of texture
 */
double painty::BrushStrokeSample::getSampleAtWarped(const vec2& uv) const
{
  return getSampleAt(_warper.warp(uv));
}

void painty::BrushStrokeSample::loadSample(const std::string& sampleDir)
{
  _txy_l.clear();
  _txy_c.clear();
  _txy_r.clear();

  _puv_l.clear();
  _puv_c.clear();
  _puv_r.clear();

  nlohmann::json json;
  {
    std::ifstream i(sampleDir + "/spine.json");
    i >> json;
    i.close();
  }
  {
    const auto arr_xy = json["txy_l"];
    const auto arr_uv = json["puv_l"];

    for (auto i = 0U; i < arr_xy.size(); i++)
    {
      vec2 xy = { arr_xy[i][0].get<double>(), arr_xy[i][1].get<double>() };
      vec2 uv = { arr_uv[i][0].get<double>(), arr_uv[i][1].get<double>() };
      addCorr_l(xy, uv);
    }
  }
  {
    const auto arr_xy = json["txy_c"];
    const auto arr_uv = json["puv_c"];

    for (auto i = 0U; i < arr_xy.size(); i++)
    {
      vec2 xy = { arr_xy[i][0].get<double>(), arr_xy[i][1].get<double>() };
      vec2 uv = { arr_uv[i][0].get<double>(), arr_uv[i][1].get<double>() };
      addCorr_c(xy, uv);
    }
  }
  {
    const auto arr_xy = json["txy_r"];
    const auto arr_uv = json["puv_r"];

    for (auto i = 0U; i < arr_xy.size(); i++)
    {
      vec2 xy = { arr_xy[i][0].get<double>(), arr_xy[i][1].get<double>() };
      vec2 uv = { arr_uv[i][0].get<double>(), arr_uv[i][1].get<double>() };
      addCorr_r(xy, uv);
    }
  }

  // create texture warper from spine
  {
    std::vector<vec2> uv;
    for (auto p : _puv_l)
    {
      uv.push_back(p);
    }
    uv.insert(uv.end(), _puv_r.rbegin(), _puv_r.rend());
    std::vector<vec2> t;
    for (auto p : _txy_l)
    {
      t.push_back(p);
    }
    t.insert(t.end(), _txy_r.rbegin(), _txy_r.rend());
    _warper.init(uv, t);
  }

  // load thickness map
  io::imRead(sampleDir + "/thickness_map.png", _thickness_map);
}
