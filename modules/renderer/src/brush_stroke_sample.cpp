/**
 * @file brush_stroke_sample.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-01
 */
#include <painty/io/image_io.h>
#include <painty/renderer/brush_stroke_sample.h>

#include <fstream>
#include <iostream>

painty::BrushStrokeSample::BrushStrokeSample(const std::string& sampleDir) {
  loadSample(sampleDir);
}

void painty::BrushStrokeSample::addCorr_l(const vec2& texPos, const vec2& uv) {
  _txy_l.push_back(texPos);
  _puv_l.push_back(uv);
}

void painty::BrushStrokeSample::addCorr_c(const vec2& texPos, const vec2& uv) {
  _txy_c.push_back(texPos);
  _puv_c.push_back(uv);
}

void painty::BrushStrokeSample::addCorr_r(const vec2& texPos, const vec2& uv) {
  _txy_r.push_back(texPos);
  _puv_r.push_back(uv);
}

const painty::Mat<double>& painty::BrushStrokeSample::getThicknessMap() const {
  return _thickness_map;
}

void painty::BrushStrokeSample::setThicknessMap(
  const Mat<double>& thicknessMap) {
  _thickness_map = thicknessMap;
}

/**
 * @brief Sample the brush stroke texture at xy.
 *
 * @param xy
 *
 * @return double 0.0 if xy outside of texture
 */
double painty::BrushStrokeSample::getSampleAt(const vec2& xy) const {
  // outside of texture
  if (xy[0] < 0.0 || xy[1] < 0.0 || xy[0] >= _thickness_map.cols ||
      xy[1] >= _thickness_map.rows) {
    return 0.0;
  }

  // get texture value using bilinear interpolation
  return Interpolate(_thickness_map, xy);
}

/**
 * @brief Sample the brush stroke texture at a canvas coordinate uv. The uv is warped implicitly to stroke sample space.
 *  u : {0.0, 1.0}
 *  v : {-1.0, 1.0}
 * @param uv
 *
 * @return double 0.0 if xy outside of texture
 */
double painty::BrushStrokeSample::getSampleAtUV(const vec2& uv) const {
  return getSampleAt(_warper.warp(uv));
}

/**
 * @brief
 *
 * @return double
 */
double painty::BrushStrokeSample::getWidth() const {
  return _widthMax;
}

/**
 * @brief
 *
 * @return double
 */
double painty::BrushStrokeSample::getLength() const {
  return _length;
}

/**
 * @brief
 *
 * @param sampleDir
 */
void painty::BrushStrokeSample::loadSample(const std::string& sampleDir) {
  _txy_l.clear();
  _txy_c.clear();
  _txy_r.clear();

  _puv_l.clear();
  _puv_c.clear();
  _puv_r.clear();

  std::ifstream ifile(sampleDir + "/spine.txt");

  std::string line;

  auto* points = &_txy_l;
  while (std::getline(ifile, line)) {
    if (line == "txy_l") {
      points = &_txy_l;
    } else if (line == "txy_c") {
      points = &_txy_c;
    } else if (line == "txy_r") {
      points = &_txy_r;
    } else if (line == "puv_l") {
      points = &_puv_l;
    } else if (line == "puv_c") {
      points = &_puv_c;
    } else if (line == "puv_r") {
      points = &_puv_r;
    } else if (line.empty()) {
      continue;
    } else {
      const auto e     = line.find_first_of(' ');
      const auto x_str = line.substr(0U, e);
      const auto y_str = line.substr(e + 1U, line.size());
      const auto x     = static_cast<double>(std::stod(x_str));
      const auto y     = static_cast<double>(std::stod(y_str));

      points->push_back({x, y});
    }
  }
  ifile.close();

  // create texture warper from spine
  {
    std::vector<vec2> uv;
    for (auto p : _puv_l) {
      uv.push_back(p);
    }
    uv.insert(uv.end(), _puv_r.rbegin(), _puv_r.rend());
    std::vector<vec2> t;
    for (auto p : _txy_l) {
      t.push_back(p);
    }
    t.insert(t.end(), _txy_r.rbegin(), _txy_r.rend());
    _warper.init(uv, t);
  }

  // load thickness map
  io::imRead(sampleDir + "/thickness_map.png", _thickness_map);

  // scan through points and find the maximum width
  _widthMax = 0.0;
  for (auto l = _txy_l.cbegin(), r = _txy_r.cbegin();
       (r != _txy_r.cend()) && (l != _txy_l.cend()); l++, r++) {
    _widthMax = std::max(_widthMax, ((*l) - (*r)).squaredNorm());
  }
  _widthMax = std::sqrt(_widthMax);
}
