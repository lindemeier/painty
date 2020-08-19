/**
 * @file superpixel.cpp
 * @author Thomas Lindemeier
 * @brief
 *
 * @date 2020-07-31
 *
 *
 */
#include "painty/image/superpixel.h"

#include <random>

#include "painty/core/color.h"

namespace painty {

ImageRegion::ImageRegion() : label(), active(false) {}

ImageRegion::ImageRegion(const int32_t labelArg, const Mat1i& labelsMap)
    : label(labelArg),
      active(true) {
  for (int32_t x = 0; x < labelsMap.cols; x++) {
    for (int32_t y = 0; y < labelsMap.rows; y++) {
      if (labelsMap(y, x) == label) {
        points.push_back(vec2i(x, y));
      }
    }
  }
}

void ImageRegion::setActive(bool activeArg) {
  this->active = activeArg;
}

bool ImageRegion::isActive() {
  return active;
}

int32_t ImageRegion::getLabel() const {
  return label;
}

void ImageRegion::setLabel(const int32_t inlabel) {
  this->label = inlabel;
}

std::vector<vec2i>::const_iterator ImageRegion::cbegin() {
  return points.cbegin();
}

std::vector<vec2i>::const_iterator ImageRegion::cend() {
  return points.cend();
}

double ImageRegion::getInscribedCircle(vec2& incenter) const {
  cv::Rect2i bound = getBoundingRectangle();

  // need to pad 1 pix for dist transform to work correctly
  const int32_t pad = 10;
  bound.x -= pad;
  bound.y -= pad;
  bound.width += pad * 2;
  bound.height += pad * 2;

  Mat1d distances = getDistanceTransform(bound);

  // find max in distance
  double maxDist = 0.0;
  for (const auto& p : points) {
    double d = distances(p[1] - bound.y, p[0] - bound.x);
    if (d > maxDist) {
      maxDist     = d;
      incenter[0] = p[0];
      incenter[1] = p[1];
    }
  }

  // find radius
  vec2 imP(incenter[0] - bound.x, incenter[1] - bound.y);
  double mindDist = std::numeric_limits<double>::max();
  for (int32_t x = 0; x < distances.cols; x++) {
    for (int32_t y = 0; y < distances.rows; y++) {
      if (distances(y, x) <= 0.0) {
        double d = (imP - vec2(x, y)).norm();
        if (d < mindDist) {
          mindDist = d;
        }
      }
    }
  }
  // subtract 1 since we found the distance to an outside pixel
  mindDist -= 1;

  return mindDist;
}

cv::Rect2i ImageRegion::getBoundingRectangle() const {
  return cv::boundingRect(points);
}

Mat1d ImageRegion::getDistanceTransform(
  const cv::Rect2i& boundingRectangle) const {
  Mat1u seg(boundingRectangle.size(), 0);
  for (const auto& p : points) {
    seg(p[1] - boundingRectangle.y, p[0] - boundingRectangle.x) = 255;
  }
  Mat1d distances(boundingRectangle.size());
  cv::distanceTransform(seg, distances, cv::NORM_L1, 5);
  return distances;
}

vec2 ImageRegion::getSpatialMean() const {
  vec2 mean = vec2::Zero();

  if (points.empty())
    return mean;

  for (const vec2i& p : points) {
    mean[0] += p[0];
    mean[1] += p[1];
  }
  return vec2(mean[0] / points.size(), mean[1] / points.size());
}

void SuperpixelSegmentation::extract(const Mat3d& targetLabArg,
                                     const Mat3d& canvasLabArg,
                                     const Mat1d& maskArg,
                                     const int32_t& brushSize) {
  _targetLab = targetLabArg;
  _mask      = maskArg;

  _difference = Mat1d(_targetLab.size());
  for (int32_t i = 0; i < static_cast<int32_t>(_targetLab.total()); i++) {
    _difference(i) =
      ColorConverter<double>::ColorDifference(_targetLab(i), canvasLabArg(i));
  }

  const int32_t N = _targetLab.cols * _targetLab.rows;
  const int32_t K = static_cast<int32_t>(N / (std::pow(brushSize, 2)));
  const int32_t S = static_cast<int32_t>(std::sqrt(N / K));

  _superPixels.clear();

  if (_extractionStrategy == SLICO_POISSON_WEIGHTED) {
    //poisson disc distribution weighted by distribution energy
    std::vector<vec2> samples;
    Mat<double> p = _difference.clone();

    std::vector<double> intervals(p.total());
    for (size_t i = 0; i < intervals.size(); ++i) {
      intervals[i] = static_cast<double>(i);
    }
    std::piecewise_linear_distribution<double> distribution =
      std::piecewise_linear_distribution<double>(intervals.begin(),
                                                 intervals.end(), p.begin());
    const auto maxSamples = K;
    samples.reserve(static_cast<size_t>(maxSamples));
    std::default_random_engine generator;
    vec2 sample;
    bool stop = false;
    for (int32_t j = 0; j < maxSamples && !stop; ++j) {
      int32_t index;
      int32_t nrTrials = 0;
      do {
        index = static_cast<int32_t>(distribution(generator));
        if (nrTrials++ >= 1000) {
          stop = true;
        }
      } while (p(index) <= 0.0);

      if (p(index) > 0.0) {
        sample << index % p.cols, index / p.cols;

        if (_mask(index) == 1.0) {
          // add poisson disc
          cv::circle(p,
                     cv::Point(static_cast<int32_t>(sample[0]),
                               static_cast<int32_t>(sample[1])),
                     S / 2, cv::Scalar(0.0), -1, 8);
          samples.push_back(sample);
        }
      }
    }
    _superPixels.reserve(samples.size());
    for (size_t k = 0; k < samples.size(); ++k) {
      _superPixels.emplace_back(
        samples[k], _targetLab(static_cast<int32_t>(samples[k][1]),
                               static_cast<int32_t>(samples[k][0])));
    }
  } else if (_extractionStrategy == SLICO_GRID) {
    for (int32_t x = S; x < _targetLab.cols; x += S) {
      for (int32_t y = S; y < _targetLab.rows; y += S) {
        vec2 sample = {static_cast<double>(x), static_cast<double>(y)};
        _superPixels.emplace_back(sample,
                                  _targetLab(static_cast<int32_t>(sample[1]),
                                             static_cast<int32_t>(sample[0])));
      }
    }
  } else if (_extractionStrategy == GRID_SHUFFLED) {
    std::default_random_engine generator;
    _superPixels.reserve(static_cast<size_t>(K));
    _labels    = Mat1i(_targetLab.size(), 0);
    int32_t la = 0;
    int32_t wG = _targetLab.cols;
    int32_t hG = _targetLab.rows;

    const int32_t cellSize0 = static_cast<int32_t>(S * 1.5);
    const int32_t cellSize1 = S;

    for (int32_t x = 0; x < wG; x += cellSize0) {
      for (int32_t y = 0; y < hG; y += cellSize0) {
        std::uniform_int_distribution<int32_t> dx(x, x + cellSize0 - cellSize1);
        std::uniform_int_distribution<int32_t> dy(y, y + cellSize0 - cellSize1);

        int32_t xs  = ::std::clamp(dx(generator), 0, wG - 1);
        int32_t ys  = ::std::clamp(dy(generator), 0, hG - 1);
        int32_t xs2 = ::std::min(xs + cellSize1, wG - 1);
        int32_t ys2 = ::std::min(ys + cellSize1, hG - 1);

        vec2 sample = {x + 0.5f * S, y + 0.5f * S};
        _superPixels.emplace_back(sample,
                                  _targetLab(static_cast<int32_t>(sample[1]),
                                             static_cast<int32_t>(sample[0])));
        for (int32_t i = ys; i < ys2; i++) {
          for (int32_t j = xs; j < xs2; j++) {
            _labels(i, j) = la;
          }
        }
        la++;
      }
    }
    computeStats(_superPixels, _labels);
    return;
  }

  // push centers away from edges
  perturbClusterCenters(_superPixels);

  // run the segmentation algorithm
  Mat<int32_t> newLabels(_targetLab.size(), -1);
  Mat1d distances(_targetLab.size(), std::numeric_limits<double>::max());

  double error = std::numeric_limits<double>::max();

  int32_t iteration = 0;
  while (error > 0.001 && iteration++ < 100) {
    for (auto& c : _superPixels) {
      c.reset();
    }

    const int32_t size = 2 * S;
    for (size_t i = 0; i < _superPixels.size(); i++) {
      SuperPixel& cluster = _superPixels[i];

      vec2i can;
      double cdist;
      double ndist;

      for (int32_t x = static_cast<int32_t>(cluster.center[0]) - size;
           x <= static_cast<int32_t>(cluster.center[0]) + size; x++) {
        for (int32_t y = static_cast<int32_t>(cluster.center[1]) - size;
             y <= static_cast<int32_t>(cluster.center[1]) + size; y++) {
          if (x < 0 || y < 0 || y >= distances.rows || x >= distances.cols)
            continue;

          if (_mask(y, x) < 1.0 || _difference(y, x) <= 0.0) {
            newLabels(y, x) = -1;
            distances(y, x) = std::numeric_limits<double>::max();
            continue;
          }

          can[0] = x;
          can[1] = y;
          cdist  = distances(y, x);
          ndist  = distance(cluster, can);

          if (ndist < cdist) {
            distances(y, x) = ndist;
            newLabels(y, x) = static_cast<int32_t>(i);
          }
        }
      }
    }
    error = computeStats(_superPixels, newLabels);
  }
  _labels = Mat<int32_t>(_targetLab.size());
  int32_t newK;
  enforceLabelConnectivity((int32_t*)newLabels.data, newLabels.cols,
                           newLabels.rows, (int32_t*)_labels.data, newK,
                           (int32_t)(double(N) / double(S * S)));
}

void SuperpixelSegmentation::getSegmentationOutlined(Mat3d& background) const {
  Mat1i labelClone = _labels.clone();

  drawContoursAroundSegments(reinterpret_cast<vec3*>(background.data),
                             reinterpret_cast<const int32_t*>(labelClone.data),
                             _labels.cols, _labels.rows, vec3(0.5f, 0.5f, 0.0));
}

void SuperpixelSegmentation::perturbClusterCenters(
  std::vector<SuperpixelSegmentation::SuperPixel>& superPixels) const {
  const int32_t r = 1;
  for (auto& cluster : superPixels) {
    cluster.area = 0;

    double minG = std::numeric_limits<double>::max();
    vec2 o;
    o[0] = cluster.center[0];
    o[1] = cluster.center[1];
    for (int32_t x_ = static_cast<int32_t>(o[0]) - r;
         x_ <= static_cast<int32_t>(o[0]) + r; x_++) {
      for (int32_t y_ = static_cast<int32_t>(o[1]) - r;
           y_ <= static_cast<int32_t>(o[1]) + r; y_++) {
        if (_mask(y_, x_) < 1.0)
          continue;

        int32_t x =
          std::min<int32_t>(_targetLab.cols - 1, std::max<int32_t>(0, x_));
        int32_t y =
          std::min<int32_t>(_targetLab.rows - 1, std::max<int32_t>(0, y_));

        int32_t px =
          cv::borderInterpolate(x - 1, _targetLab.cols, cv::BORDER_REFLECT);
        int32_t nx =
          cv::borderInterpolate(x + 1, _targetLab.cols, cv::BORDER_REFLECT);
        int32_t py =
          cv::borderInterpolate(y - 1, _targetLab.rows, cv::BORDER_REFLECT);
        int32_t ny =
          cv::borderInterpolate(y + 1, _targetLab.rows, cv::BORDER_REFLECT);

        double G = (_targetLab(y, nx) - _targetLab(y, px)).norm() +
                   (_targetLab(ny, x) - _targetLab(py, x)).norm();

        if (G < minG) {
          minG              = G;
          cluster.center[0] = x;
          cluster.center[1] = y;
          cluster.meanColor = _targetLab(y, x);
        }
      }
    }
  }
}

double SuperpixelSegmentation::computeStats(
  std::vector<SuperpixelSegmentation::SuperPixel>& superPixels,
  Mat<int32_t>& labels) const {
  for (int32_t x = 0; x < labels.cols; ++x) {
    for (int32_t y = 0; y < labels.rows; ++y) {
      int32_t clusterID = labels(y, x);
      if (_mask(y, x) < 1.0 || clusterID == -1)
        continue;

      SuperPixel& center = superPixels[static_cast<size_t>(clusterID)];
      center.meanColorT += _targetLab(y, x);
      center.meanDiffT += _difference(y, x);
      center.centerT[0] += x;
      center.centerT[1] += y;
      center.area++;
    }
  }

  double error = 0.;
  for (auto& superPixel : superPixels) {
    if (superPixel.area == 0)
      continue;

    const double f = 1.0 / superPixel.area;
    superPixel.meanColorT *= f;
    superPixel.meanDiffT *= f;
    superPixel.centerT *= f;

    error += (superPixel.centerT - superPixel.center).norm() +
             (superPixel.meanColor - superPixel.meanColorT).norm();
    superPixel.center         = superPixel.centerT;
    superPixel.meanColor      = superPixel.meanColorT;
    superPixel.meanDiff       = superPixel.meanDiffT;
    superPixel.maxColorDist   = superPixel.maxColorDistT;
    superPixel.maxDiffDist    = superPixel.maxDiffDistT;
    superPixel.maxSpatialDist = superPixel.maxSpatialDistT;
  }

  return error / superPixels.size();
}

double SuperpixelSegmentation::distance(
  SuperpixelSegmentation::SuperPixel& superPixel, const vec2i& pos2) const {
  if (_useDiffWeight) {
    const double dc =
      (superPixel.meanColor - _targetLab(pos2[1], pos2[0])).norm();
    const double ds = (superPixel.center - vec2(pos2[0], pos2[1])).norm();
    const double dd = std::sqrt(
      std::pow(superPixel.meanDiff - _difference(pos2[1], pos2[0]), 2.0));

    superPixel.maxColorDistT   = std::max(superPixel.maxColorDistT, dc);
    superPixel.maxDiffDistT    = std::max(superPixel.maxDiffDistT, dd);
    superPixel.maxSpatialDistT = std::max(superPixel.maxSpatialDistT, ds);
    // SLICO
    return std::sqrt(std::pow(dc / superPixel.maxColorDist, 2.0f) +
                     std::pow(dd / superPixel.maxDiffDist, 2.0f) +
                     std::pow(ds / superPixel.maxSpatialDist, 2.0f));
  } else {
    const double dc =
      (superPixel.meanColor - _targetLab(pos2[1], pos2[0])).norm();
    const double ds = (superPixel.center - vec2(pos2[0], pos2[1])).norm();

    superPixel.maxColorDistT   = std::max(superPixel.maxColorDistT, dc);
    superPixel.maxSpatialDistT = std::max(superPixel.maxSpatialDistT, ds);
    // SLICO
    return std::sqrt(std::pow(dc / superPixel.maxColorDist, 2.0f) +
                     std::pow(ds / superPixel.maxSpatialDist, 2.0f));
  }
}

const Mat1i& SuperpixelSegmentation::getRegions(
  std::map<int32_t, ImageRegion>& regions) {
  std::vector<std::pair<ImageRegion, double>> regionsS;
  for (int32_t label = 0; label < static_cast<int32_t>(_superPixels.size());
       label++) {
    ImageRegion r(label, _labels);
    double m = r.computeMean(_difference);
    regionsS.emplace_back(std::move(r), m);
  }

  //    std::sort(regionsS.begin(), regionsS.end(), [](std::pair<ImageRegion, double> &a, std::pair<ImageRegion, double> &b) {
  //        return std::get<1>(a) > std::get<1>(b);
  //    });

  std::random_shuffle(regionsS.begin(), regionsS.end());

  regions.clear();
  int32_t newLabel = 0;
  for (auto& t : regionsS) {
    ImageRegion& region = std::get<0>(t);
    region.setLabel(newLabel++);
    region.fill(_labels, region.getLabel());
    regions.emplace(std::make_pair(region.getLabel(), std::move(region)));
  }
  return _labels;
}

void SuperpixelSegmentation::setExtractionStrategy(
  SuperpixelSegmentation::ExtractionStrategy extractionStrategy) {
  _extractionStrategy = extractionStrategy;
}

void SuperpixelSegmentation::setUseDiffWeight(bool useDiffWeight) {
  _useDiffWeight = useDiffWeight;
}

}  // namespace painty
