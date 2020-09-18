/**
 * @file Superpixel.cxx
 * @author Thomas Lindemeier
 * @brief
 *
 * @date 2020-07-31
 *
 *
 */
#include "painty/image/Superpixel.hxx"

#include <random>

#include "math.h"
#include "painty/core/Color.hxx"

namespace segmentation_details {
static void drawContoursAroundSegments(painty::Mat3d& image,
                                       const painty::Mat<int32_t>& labels,
                                       const painty::vec3& color) {
  const std::array<int32_t, 8UL> dx8 = {-1, -1, 0, 1, 1, 1, 0, -1};
  const std::array<int32_t, 8UL> dy8 = {0, -1, -1, -1, 0, 1, 1, 1};

  const auto sz = image.total();

  std::vector<bool> istaken(sz, false);

  auto mainindex = 0;
  for (int32_t j = 0; j < image.rows; j++) {
    for (int32_t k = 0; k < image.cols; k++) {
      int32_t np(0);
      for (int32_t i = 0; i < 8; i++) {
        int32_t x = k + dx8[static_cast<size_t>(i)];
        int32_t y = j + dy8[static_cast<size_t>(i)];

        if ((x >= 0 && x < image.cols) && (y >= 0 && y < image.rows)) {
          const auto index = y * image.cols + x;

          if (labels(index) < 0) {
            continue;
          }

          if (!istaken[static_cast<size_t>(index)]) {
            if (labels(mainindex) != labels(index)) {
              np++;
            }
          }
        }
      }
      if (np > 1) {
        image(mainindex)                        = color;
        istaken[static_cast<size_t>(mainindex)] = true;
      }
      mainindex++;
    }
  }
}

static painty::Mat<int32_t> enforceLabelConnectivity(
  painty::Mat<int32_t>& labels, int32_t& numlabels, int32_t K) {
  const std::array<int32_t, 4UL> xn4 = {-1, 0, 1, 0};
  const std::array<int32_t, 4UL> yn4 = {0, -1, 0, 1};

  const auto sz = labels.total();

  painty::Mat<int32_t> nlabels(labels.size());

  const int32_t supsz = static_cast<int32_t>(sz / static_cast<size_t>(K));

  for (auto i = 0U; i < sz; i++) {
    nlabels(static_cast<int32_t>(i)) = -1;
  }
  int32_t label = 0;
  std::vector<int32_t> x_vector(sz);
  std::vector<int32_t> y_vector(sz);
  int32_t oindex   = 0;
  int32_t adjlabel = 0;
  for (int32_t j = 0; j < labels.rows; j++) {
    for (int32_t k = 0; k < labels.cols; k++) {
      if (0 > nlabels(oindex)) {
        nlabels(oindex) = label;

        x_vector[0] = k;
        y_vector[0] = j;

        for (auto n = 0UL; n < 4UL; n++) {
          int32_t x = x_vector[0] + xn4[n];
          int32_t y = y_vector[0] + yn4[n];
          if ((x >= 0 && x < labels.cols) && (y >= 0 && y < labels.rows)) {
            int32_t nindex = y * labels.cols + x;
            if (nlabels(nindex) >= 0) {
              adjlabel = nlabels(nindex);
            }
          }
        }

        int32_t count = 1;
        for (int32_t c = 0; c < count; c++) {
          for (auto n = 0UL; n < 4UL; n++) {
            int32_t x = x_vector[static_cast<size_t>(c)] + xn4[n];
            int32_t y = y_vector[static_cast<size_t>(c)] + yn4[n];

            if ((x >= 0 && x < labels.cols) && (y >= 0 && y < labels.rows)) {
              int32_t nindex = y * labels.cols + x;

              if (0 > nlabels(nindex) && labels(oindex) == labels(nindex)) {
                x_vector[static_cast<size_t>(count)] = x;
                y_vector[static_cast<size_t>(count)] = y;
                nlabels(nindex)                      = label;
                count++;
              }
            }
          }
        }

        if (count <= supsz >> 2) {
          for (int32_t c = 0; c < count; c++) {
            int32_t ind = y_vector[static_cast<size_t>(c)] * labels.cols +
                          x_vector[static_cast<size_t>(c)];
            nlabels(ind) = adjlabel;
          }
          label--;
        }
        label++;
      }
      oindex++;
    }
  }
  numlabels = label;

  return nlabels;
}

}  // namespace segmentation_details

namespace painty {

constexpr auto EpsMask = (std::numeric_limits<double>::epsilon() * 100.0);

SuperpixelSegmentation::SuperPixel::SuperPixel(const vec2& center,
                                               const vec3& meanColor)
    : _center(center),

      _meanColor(meanColor),

      _meanDiff(),
      _meanDiffT(),
      _area(),
      _maxSpatialDist(1.0),
      _maxSpatialDistT(),
      _maxColorDist(1.0),
      _maxColorDistT(),
      _maxDiffDist(0.001),
      _maxDiffDistT() {
  reset();
}

SuperpixelSegmentation::SuperPixel::SuperPixel()
    : _meanDiff(),
      _meanDiffT(),
      _area(),
      _maxSpatialDist(1.0),
      _maxSpatialDistT(),
      _maxColorDist(1.0),
      _maxColorDistT(),
      _maxDiffDist(0.001),
      _maxDiffDistT() {
  reset();
}

void SuperpixelSegmentation::SuperPixel::reset() {
  _centerT         = {0.0, 0.0};
  _meanColorT      = vec3::Zero();
  _meanDiffT       = 0.0;
  _maxColorDistT   = 0.0001;
  _maxDiffDistT    = 0.0001;
  _maxSpatialDistT = 0.0001;
  _area            = 0;
}

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

bool ImageRegion::isActive() const {
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
  Mat1f distances(boundingRectangle.size());
  cv::distanceTransform(seg, distances, cv::NORM_L1, 5);
  Mat1d distancesScaled;
  distances.convertTo(distancesScaled, CV_64FC1);
  return distancesScaled;
}

vec2 ImageRegion::getSpatialMean() const {
  vec2 mean = vec2::Zero();

  if (points.empty()) {
    return mean;
  }

  for (const vec2i& p : points) {
    mean[0] += p[0];
    mean[1] += p[1];
  }
  return vec2(mean[0] / static_cast<double>(points.size()),
              mean[1] / static_cast<double>(points.size()));
}

void SuperpixelSegmentation::extract(const Mat3d& targetLabArg,
                                     const Mat3d& canvasLabArg,
                                     const Mat1d& maskArg,
                                     const int32_t cellWidth) {
  _targetLab = targetLabArg;
  _mask =
    (maskArg.empty()) ? Mat1d(_targetLab.rows, _targetLab.cols, 1.0) : maskArg;

  if (!canvasLabArg.empty()) {
    _useDiffWeight = true;
    _difference    = Mat1d(_targetLab.size());
    for (int32_t i = 0; i < static_cast<int32_t>(_targetLab.total()); i++) {
      _difference(i) =
        ColorConverter<double>::ColorDifference(_targetLab(i), canvasLabArg(i));
    }
  } else {
    _useDiffWeight = false;
  }

  const int32_t N = _targetLab.cols * _targetLab.rows;
  const int32_t K = static_cast<int32_t>(N / (std::pow(cellWidth, 2)));
  const int32_t S = static_cast<int32_t>(std::sqrt(N / K));

  _superPixels.clear();

  if ((!_difference.empty()) &&
      (_extractionStrategy == SLICO_POISSON_WEIGHTED)) {
    //poisson disc distribution weighted by distribution energy
    std::vector<vec2> samples;
    Mat1d p = _difference.clone();

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
    for (int32_t j = 0; (j < maxSamples) && (!stop); ++j) {
      int32_t index    = 0;
      int32_t nrTrials = 0;
      do {
        index = static_cast<int32_t>(distribution(generator));
        if (nrTrials++ >= 1000) {
          stop = true;
        }
      } while (p(index) <= 0.0);

      if (p(index) > 0.0) {
        sample << index % p.cols, index / p.cols;

        if (_mask(index) > 0.0) {
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
  } else {  //(_extractionStrategy == GRID_SHUFFLED)
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

        vec2 sample = {static_cast<double>(x + S / 2),
                       static_cast<double>(y + S / 2)};
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

  auto error                   = std::numeric_limits<double>::max();
  auto iteration               = 0U;
  constexpr auto MaxIterations = 100U;
  constexpr auto MaxError      = 0.001;
  while ((error > MaxError) && (iteration++ < MaxIterations)) {
    for (auto& c : _superPixels) {
      c.reset();
    }

    const int32_t size = 2 * S;
    for (size_t i = 0; i < _superPixels.size(); i++) {
      SuperPixel& cluster = _superPixels[i];

      vec2i can;
      double cdist = 0.0;
      double ndist = 0.0;

      for (int32_t x = static_cast<int32_t>(cluster._center[0]) - size;
           x <= static_cast<int32_t>(cluster._center[0]) + size; x++) {
        for (int32_t y = static_cast<int32_t>(cluster._center[1]) - size;
             y <= static_cast<int32_t>(cluster._center[1]) + size; y++) {
          if (x < 0 || y < 0 || y >= distances.rows || x >= distances.cols) {
            continue;
          }

          if (fuzzyCompare(_mask(y, x), 0.0, EpsMask) ||
              (!_difference.empty() && (_difference(y, x) <= 0.0))) {
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
  int32_t newK = 0;
  _labels      = segmentation_details::enforceLabelConnectivity(
    newLabels, newK, static_cast<int32_t>(double(N) / double(S * S)));
}

void SuperpixelSegmentation::getSegmentationOutlined(Mat3d& background) const {
  segmentation_details::drawContoursAroundSegments(background, _labels,
                                                   vec3(0.5, 0.5, 0.0));
}

void SuperpixelSegmentation::perturbClusterCenters(
  std::vector<SuperpixelSegmentation::SuperPixel>& superPixels) const {
  const int32_t r = 1;
  for (auto& cluster : superPixels) {
    cluster._area = 0;

    double minG = std::numeric_limits<double>::max();
    vec2 o;
    o[0] = cluster._center[0];
    o[1] = cluster._center[1];
    for (int32_t x_ = static_cast<int32_t>(o[0]) - r;
         x_ <= static_cast<int32_t>(o[0]) + r; x_++) {
      for (int32_t y_ = static_cast<int32_t>(o[1]) - r;
           y_ <= static_cast<int32_t>(o[1]) + r; y_++) {
        if ((x_ < 0) || (x_ >= _targetLab.cols) || (y_ < 0) ||
            (y_ >= _targetLab.rows)) {
          continue;
        }
        if (fuzzyCompare(_mask(y_, x_), 0.0, EpsMask)) {
          continue;
        }

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
          minG               = G;
          cluster._center[0] = x;
          cluster._center[1] = y;
          cluster._meanColor = _targetLab(y, x);
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
      if (fuzzyCompare(_mask(y, x), 0.0, EpsMask) || clusterID == -1) {
        continue;
      }

      SuperPixel& center = superPixels[static_cast<size_t>(clusterID)];
      center._meanColorT += _targetLab(y, x);
      center._meanDiffT += (_difference.empty()) ? 0.0 : _difference(y, x);
      center._centerT[0] += x;
      center._centerT[1] += y;
      center._area++;
    }
  }

  double error = 0.;
  for (auto& superPixel : superPixels) {
    if (superPixel._area == 0) {
      continue;
    }

    const double f = 1.0 / superPixel._area;
    superPixel._meanColorT *= f;
    superPixel._meanDiffT *= f;
    superPixel._centerT *= f;

    error += (superPixel._centerT - superPixel._center).norm() +
             (superPixel._meanColor - superPixel._meanColorT).norm();
    superPixel._center         = superPixel._centerT;
    superPixel._meanColor      = superPixel._meanColorT;
    superPixel._meanDiff       = superPixel._meanDiffT;
    superPixel._maxColorDist   = superPixel._maxColorDistT;
    superPixel._maxDiffDist    = superPixel._maxDiffDistT;
    superPixel._maxSpatialDist = superPixel._maxSpatialDistT;
  }

  return error / static_cast<double>(superPixels.size());
}

double SuperpixelSegmentation::distance(
  SuperpixelSegmentation::SuperPixel& superPixel, const vec2i& pos2) const {
  if (_useDiffWeight) {
    const double dc =
      (superPixel._meanColor - _targetLab(pos2[1], pos2[0])).norm();
    const double ds = (superPixel._center - vec2(pos2[0], pos2[1])).norm();
    const double dd =
      (_difference.empty())
        ? 0.0
        : std::sqrt(std::pow(
            superPixel._meanDiff - _difference(pos2[1], pos2[0]), 2.0));

    superPixel._maxColorDistT   = std::max(superPixel._maxColorDistT, dc);
    superPixel._maxDiffDistT    = std::max(superPixel._maxDiffDistT, dd);
    superPixel._maxSpatialDistT = std::max(superPixel._maxSpatialDistT, ds);
    // SLICO
    return std::sqrt(std::pow(dc / superPixel._maxColorDist, 2.0F) +
                     std::pow(dd / superPixel._maxDiffDist, 2.0F) +
                     std::pow(ds / superPixel._maxSpatialDist, 2.0F));
  }
  const double dc =
    (superPixel._meanColor - _targetLab(pos2[1], pos2[0])).norm();
  const double ds = (superPixel._center - vec2(pos2[0], pos2[1])).norm();

  superPixel._maxColorDistT   = std::max(superPixel._maxColorDistT, dc);
  superPixel._maxSpatialDistT = std::max(superPixel._maxSpatialDistT, ds);
  // SLICO
  return std::sqrt(std::pow(dc / superPixel._maxColorDist, 2.0) +
                   std::pow(ds / superPixel._maxSpatialDist, 2.0));
}

const Mat1i& SuperpixelSegmentation::getRegions(
  std::map<int32_t, ImageRegion>& regions) {
  std::vector<std::pair<ImageRegion, double>> regionsS;
  for (int32_t label = 0; label < static_cast<int32_t>(_superPixels.size());
       label++) {
    ImageRegion r(label, _labels);
    double m = (_difference.empty()) ? 0.0 : r.computeMean(_difference);
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
