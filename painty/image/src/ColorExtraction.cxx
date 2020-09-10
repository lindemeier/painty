/**
 * @file ColorExtraction.hxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2019-04-14
 */
#include "painty/image/ColorExtraction.hxx"

#include <opencv2/imgproc.hpp>

namespace painty {
/**
 * Pigment-Based Recoloring of Watercolor Paintings -
 * Elad Aharoni-Mack, Yakov Shambik and Dani Lischinski -
 * Expressive 2017
 * @param sRGB input picture in srgb space
 * @param colors output vector with RGB colors
 * @param k number of colors
 */
void ExtractColorPaletteAharoni(const Mat<vec3>& sRGB,
                                std::vector<vec3>& linearRGB_colors,
                                uint32_t k) {
  // https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
  const auto DistanceLinePoint = [](const vec2& v, const vec2& w,
                                    const vec2& p) {
    // Return minimum distance between line segment vw and point p
    const auto l2 = (v - w).squaredNorm();  // i.e. |w-v|^2 -  avoid a sqrt
    if (std::fabs(l2) < std::numeric_limits<double>::epsilon()) {
      return (p - v).norm();  // v == w case
    }

    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of point p onto the line.
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    // We clamp t from [0,1] to handle points outside the segment vw.
    const auto t =
      std::max<double>(0., std::min<double>(1., (p - v).dot(w - v) / l2));
    const vec2 projection = v + t * (w - v);  // Projection falls on the segment

    return (p - projection).norm();
  };

  // convert to lab space
  std::vector<vec3> Lab(sRGB.total());

  ColorConverter<double> converter;

  for (auto i = 0U; i < Lab.size(); i++) {
    converter.srgb2lab(sRGB(static_cast<int32_t>(i)), Lab[i]);
  }

  auto L_max        = 0.0;
  auto L_min        = 100.0;
  const auto L_down = 0.0;
  const auto L_up   = 0.0;
  vec3 c_max_L;
  vec3 c_min_L;
  for (const vec3& e : Lab) {
    if (L_max < e[0]) {
      L_max   = e[0];
      c_max_L = e;
    }
    if (L_min > e[0]) {
      L_min   = e[0];
      c_min_L = e;
    }
  }

  // list of colors in ab plane
  std::vector<vec2> inputPoints;
  std::vector<vec3> inputColors;
  std::vector<int32_t> indices;
  inputPoints.reserve(Lab.size());
  inputColors.reserve(Lab.size());
  indices.reserve(Lab.size());
  int32_t index = 0;
  for (const vec3& e : Lab) {
    // To avoid noise, we first discard the brightest and the darkest
    // colors, and compute the convex hull of the remaining colors in the
    // image.
    if (e[0] > (L_min + L_down) && e[0] < (L_max - L_up)) {
      inputPoints.push_back({e[1], e[2]});
      inputColors.push_back(e);
      indices.push_back(index++);
    }
  }

  indices.clear();
  cv::convexHull(inputPoints, indices, true, false);

  linearRGB_colors.reserve(static_cast<size_t>(k));
  k -= 2;
  vec3 a, b;
  converter.lab2rgb(c_min_L, a);
  linearRGB_colors.push_back(a);
  converter.lab2rgb(c_max_L, b);
  linearRGB_colors.push_back(b);

  // trivial cases
  if ((indices.size() <= k) || (k > indices.size())) {
    for (auto i : indices) {
      vec3 converted;
      converter.lab2rgb(inputColors[static_cast<size_t>(i)], converted);
      linearRGB_colors.push_back({converted[0U], converted[1U], converted[2U]});
    }
    return;
  }
  /*
   * We then greedily iterate in order to simplify the convex hull
   * polygon by pruning its vertices until we are left with k vertices,
   * where k is the desired palette size. Pruning is done similarly to
   * the Douglas-Peucker algorithm [1973], where at each iteration
   * we remove the vertex whose distance from the line connecting
   * its neighbors is the smallest.
   * */
  while (indices.size() > k) {
    // first point
    vec2 p_1 = inputPoints[static_cast<size_t>(indices.back())];
    vec2 p0  = inputPoints[static_cast<size_t>(indices.front())];
    vec2 p1  = inputPoints[static_cast<size_t>(indices[1U])];

    int32_t removeIndex = 0;
    double md           = DistanceLinePoint(p_1, p1, p0);

    // inner points
    for (auto l = 1UL; l < indices.size() - 1; l++) {
      const int32_t i_1 = indices[l - 1];
      const int32_t i0  = indices[l];
      const int32_t i1  = indices[l + 1];

      p_1 = inputPoints[static_cast<size_t>(i_1)];
      p0  = inputPoints[static_cast<size_t>(i0)];
      p1  = inputPoints[static_cast<size_t>(i1)];

      double d = DistanceLinePoint(p_1, p1, p0);
      if (d < md) {
        md          = d;
        removeIndex = static_cast<int32_t>(l);
      }
    }

    // last point
    p_1      = inputPoints[static_cast<size_t>(*(indices.cend() - 2))];
    p0       = inputPoints[static_cast<size_t>(indices.back())];
    p1       = inputPoints[static_cast<size_t>(indices.front())];
    double d = DistanceLinePoint(p_1, p1, p0);
    if (d < md) {
      removeIndex = static_cast<int32_t>(indices.size()) - 1;
    }

    indices.erase(indices.begin() + removeIndex);
  }

  for (auto i : indices) {
    vec3 converted;
    converter.lab2rgb(inputColors[static_cast<size_t>(i)], converted);
    linearRGB_colors.push_back({converted[0U], converted[1U], converted[2U]});
  }
}
}  // namespace painty
