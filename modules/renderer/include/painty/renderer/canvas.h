/**
 * @file canvas.h
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-05-15
 *
 */
#pragma once

#include <painty/core/kubelka_munk.h>
#include <painty/core/vec.h>
#include <painty/image/mat.h>
#include <painty/renderer/paint_layer.h>

#include <algorithm>
#include <chrono>
#include <type_traits>

namespace painty {
template <class vector_type>
class Canvas final {
  using T                 = typename DataType<vector_type>::channel_type;
  static constexpr auto N = DataType<vector_type>::dim;

 public:
  Canvas(const int32_t rows, const int32_t cols)
      : _paintLayer(rows, cols),
        _backgroundColor(),
        _R0_buffer(rows, cols),
        _h_buffer(rows, cols),
        _timeMap(rows * cols),
        _dryingTime(static_cast<uint32_t>(0.25 * 60 * 1000000)) {
    _backgroundColor.fill(static_cast<T>(1.0));
    clear();
  }

  void clear() {
    _paintLayer.clear();

    const auto rows = _paintLayer.getRows();
    const auto cols = _paintLayer.getCols();
    _R0_buffer      = Mat<vector_type>(rows, cols);
    _h_buffer       = Mat<T>(rows, cols);

    _timeMap = std::vector<std::chrono::system_clock::time_point>(rows * cols);
    auto now = std::chrono::system_clock::now();
    for (auto& t : _timeMap) {
      t = now;
    }

    auto& r0 = _R0_buffer;
    auto& h  = _h_buffer;
    for (auto i = 0; i < static_cast<int32_t>(r0.total()); i++) {
      r0(i) = _backgroundColor;
      h(i)  = static_cast<T>(0.0);
    }
  }

  const Mat<vector_type>& getR0() const {
    return _R0_buffer;
  }

  const Mat<T>& get_h() const {
    return _h_buffer;
  }

  Mat<vector_type>& getR0() {
    return _R0_buffer;
  }

  Mat<T>& get_h() {
    return _h_buffer;
  }

  Mat<vector_type> getReflectanceLayerDry() const {
    return _R0_buffer.clone();
  }

  void setBackground(const Mat<vector_type>& background) {
    clear();
    auto& r0_data      = _R0_buffer;
    const auto& b_data = background;
    for (auto i = 0; i < static_cast<int32_t>(r0_data.total()); i++) {
      r0_data(i) = b_data(i);
    }
  }

  const PaintLayer<vector_type>& getPaintLayer() const {
    return _paintLayer;
  }

  PaintLayer<vector_type>& getPaintLayer() {
    return _paintLayer;
  }

  const std::vector<std::chrono::system_clock::time_point>& getTimeMap() const {
    return _timeMap;
  }

  std::vector<std::chrono::system_clock::time_point>& getTimeMap() {
    return _timeMap;
  }

  void dryCanvas() {
    const auto timePoint = std::chrono::system_clock::now();
    for (auto y = 0; y < _paintLayer.getRows(); y++) {
      for (auto x = 0; x < _paintLayer.getCols(); x++) {
        const auto v = _paintLayer.getV_buffer()(y, x);
        _h_buffer(y, x) += v;
        getR0()(y, x) =
          ComputeReflectance(_paintLayer.getK_buffer()(y, x),
                             _paintLayer.getS_buffer()(y, x), getR0()(y, x), v);
        _paintLayer.getV_buffer()(y, x) = 0.0;
        _paintLayer.getK_buffer()(y, x).fill(0.0);
        _paintLayer.getS_buffer()(y, x).fill(0.0);

        _timeMap[y * _h_buffer.cols + x] = timePoint;
      }
    }
  }

  void checkDry(int32_t x, int32_t y,
                const std::chrono::system_clock::time_point& timePoint) {
    T v = _paintLayer.getV_buffer()(y, x);
    if ((_dryingTime.count() > 0.0) && (v > 0.001)) {
      auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(
        timePoint - _timeMap[y * _h_buffer.cols + x]);

      if (dur >= _dryingTime) {
        _h_buffer(y, x) += v;
        getR0()(y, x) =
          ComputeReflectance(_paintLayer.getK_buffer()(y, x),
                             _paintLayer.getS_buffer()(y, x), getR0()(y, x), v);
        _paintLayer.getV_buffer()(y, x) = 0.0;
        _paintLayer.getK_buffer()(y, x).fill(0.0);
        _paintLayer.getS_buffer()(y, x).fill(0.0);
      } else {
        const T rate = dur.count() / static_cast<T>(_dryingTime.count());

        if (rate > 0.01) {
          // only dry a portion
          T vl = rate * v;  // amount of paint getting dry

          _h_buffer(y, x) += vl;
          T vr          = v - vl;  // amount of paint left (being wet).
          getR0()(y, x) = ComputeReflectance(_paintLayer.getK_buffer()(y, x),
                                             _paintLayer.getS_buffer()(y, x),
                                             getR0()(y, x), vl);
          _paintLayer.getV_buffer()(y, x) = vr;
        }
      }
    }
    _timeMap[y * _h_buffer.cols + x] = timePoint;
  }

  std::chrono::milliseconds getDryingTime() {
    return _dryingTime;
  }

  void setDryingTime(std::chrono::milliseconds msecs) {
    _dryingTime = msecs;
  }

 private:
  /**
   * @brief Wet paint layer.
   *
   */
  PaintLayer<vector_type> _paintLayer;
  /**
   * @brief Background color as init for substrate.
   *
   */
  vector_type _backgroundColor;
  /**
   * @brief Substrate reflectance. Layer of dried paint.
   *
   */
  Mat<vector_type> _R0_buffer;
  /**
   * @brief Height map.
   *
   */
  Mat<T> _h_buffer;

  /**
   * @brief Time pased since a cell was affected by paint.
   *
   */
  std::vector<std::chrono::system_clock::time_point> _timeMap;

  /**
   * @brief Total time of drying process.
   *
   */
  std::chrono::milliseconds _dryingTime;
};
}  // namespace painty
