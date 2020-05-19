/**
 * @file brush_stroke_sample.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-01
 *
 */
#ifndef PAINTY_BRUSH_STROKE_SAMPLE_H
#define PAINTY_BRUSH_STROKE_SAMPLE_H

#include <vector>

#include <painty/mat.h>
#include <painty/vec.h>
#include <painty/texture_warp.h>

namespace painty
{
class BrushStrokeSample final
{
public:
  BrushStrokeSample(const std::string& sampleDir);

  void addCorr_l(const vec2& texPos, const vec2& uv);
  void addCorr_c(const vec2& texPos, const vec2& uv);
  void addCorr_r(const vec2& texPos, const vec2& uv);

  const Mat<double>& getThicknessMap() const;
  void setThicknessMap(const Mat<double>& thicknessMap);

  double getSampleAt(const vec2& xy) const;
  double getSampleAtUV(const vec2& uv) const;

  void loadSample(const std::string& sampleDir);

  double getWidth() const;

private:
  Mat<double> _thickness_map;

  // texture coordinates
  std::vector<vec2> _txy_l;
  std::vector<vec2> _txy_c;
  std::vector<vec2> _txy_r;

  // parameterization
  std::vector<vec2> _puv_l;
  std::vector<vec2> _puv_c;
  std::vector<vec2> _puv_r;

  /**
   * @brief Warps positions. This is useful it the stroke sample is not straight but a curved stroke.
   */
  TextureWarp _warper;
};

}  // namespace painty

#endif  // PAINTY_BRUSH_STROKE_SAMPLE_H
