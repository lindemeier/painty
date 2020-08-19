/**
 * @file superpixel.h
 * @author Thomas Lindemeier
 * @brief
 *
 * @date 2020-07-31
 *
 *
 */
#pragma once

#include <painty/image/mat.h>

#include <map>
#include <vector>

namespace painty {
class ImageRegion {
  int32_t label;

  std::vector<vec2i> points;

  bool active;

 public:
  ImageRegion();

  ImageRegion(const int32_t label, const Mat1i& labelsMap);

  void setLabel(const int32_t label);

  void setActive(bool active);

  bool isActive();

  int32_t getLabel() const;

  vec2 getSpatialMean() const;

  std::vector<vec2i>::const_iterator cbegin();

  std::vector<vec2i>::const_iterator cend();

  double getInscribedCircle(vec2& incenter) const;

  template <class T>
  typename DataType<T>::channel_type computeRms(const Mat<T>& data0,
                                                const Mat<T>& data1) const {
    if (points.empty()) {
      return T(0.);
    }
    typename DataType<T>::channel_type s = T(0.);
    for (const vec2i& p : points) {
      s += (data0(p[1], p[0]) - data1(p[1], p[0])).squaredNorm();
    }

    return std::sqrt(s /= points.size());
  }

  template <class T>
  T computeRms(const Mat<T>& diff) const {
    if (points.empty()) {
      return T(0.);
    }
    T s = T(0.);
    for (const vec2i& p : points) {
      s += std::pow(diff(p[1], p[0]), 2.);
    }

    return std::sqrt(s /= points.size());
  }

  template <class T>
  void fill(Mat<T>& m, const T& v) const {
    if (points.empty()) {
      return;
    }
    for (const vec2i& p : points) {
      m(p[1], p[0]) = v;
    }
  }

  template <class T>
  T computeSum(const Mat<T>& v) const {
    if (points.empty()) {
      return T(0.);
    }
    T s = T(0.);
    for (const vec2i& p : points) {
      s += v(p[1], p[0]);
    }

    return s;
  }

  template <class T>
  T computeMean(const Mat<T>& data) const {
    T m = 0.;
    for (const vec2i& p : points) {
      m += data(p[1], p[0]);
    }
    return (1.0 / points.size()) * m;
  }

  template <class T, int32_t Channels>
  vec<T, Channels> computeMean(const Mat<vec<T, Channels> >& data) const {
    vec<T, Channels> m = vec<T, Channels>::Zero();
    for (const vec2i& p : points) {
      m += data(p[1], p[0]);
    }
    return (1.0 / points.size()) * m;
  }

  Mat1d getDistanceTransform(const cv::Rect2i& boundingRectangle) const;

  cv::Rect2i getBoundingRectangle() const;
};

class SuperpixelSegmentation {
  class SuperPixel {
   public:
    vec2 center;
    vec2 centerT;
    vec3 meanColor;
    vec3 meanColorT;
    double meanDiff;
    double meanDiffT;
    int32_t area;

    double maxSpatialDist;
    double maxSpatialDistT;
    double maxColorDist;
    double maxColorDistT;
    double maxDiffDist;
    double maxDiffDistT;

    void reset();

   public:
    SuperPixel();

    SuperPixel(const vec2& center, const vec3& meanColor);

    vec2 getCenter() const {
      return center;
    }

    vec3 getMeanColor() const {
      return meanColor;
    }

    int32_t getArea() const {
      return area;
    }
  };

 public:
  enum ExtractionStrategy : uint8_t {
    SLICO_GRID             = static_cast<uint8_t>(0U),
    SLICO_POISSON_WEIGHTED = static_cast<uint8_t>(1U),
    GRID                   = static_cast<uint8_t>(2U),
    GRID_SHUFFLED          = static_cast<uint8_t>(3U)
  };

 public:
  void extract(const Mat3d& targetLab, const Mat3d& canvasLab,
               const Mat1d& mask, const int32_t& brushSize);

  void getSegmentationOutlined(Mat3d& background) const;

  const Mat1i& getRegions(std::map<int32_t, ImageRegion>& regions);

  void setExtractionStrategy(ExtractionStrategy extractionStrategy);

  void setUseDiffWeight(bool useDiffWeight);

 private:
  void perturbClusterCenters(std::vector<SuperPixel>& superPixels) const;

  double computeStats(std::vector<SuperPixel>& superPixels,
                      Mat<int32_t>& labels) const;

  double distance(SuperPixel& superPixel, const vec2i& pos2) const;

 private:
  std::vector<SuperPixel> _superPixels;
  Mat3d _targetLab;
  Mat1d _difference;
  Mat<int32_t> _labels;
  Mat1d _mask;

  ExtractionStrategy _extractionStrategy =
    ExtractionStrategy::SLICO_POISSON_WEIGHTED;

  bool _useDiffWeight = true;
};

}  // namespace painty
