/**
 * @file FootprintBrush.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-29
 *
 */
#pragma once

#include <iostream>
#include <random>

#include "painty/io/ImageIO.hxx"
#include "painty/renderer/Canvas.hxx"
#include "painty/renderer/PaintLayer.hxx"

namespace painty {
template <class vector_type>
class FootprintBrush final {
  using T                 = typename DataType<vector_type>::channel_type;
  static constexpr auto N = DataType<vector_type>::dim;

 public:
  FootprintBrush(const double radius)
      : _sizeMap(0),
        _footprint(0, 0),
        _pickupMap(0, 0),
        _snapshotBuffer(0, 0)

  {
    setRadius(radius);
  }

  ~FootprintBrush() = default;

  /**
   * @brief Change the radius of the brush.
   *
   * @param radius
   */
  void setRadius(const double radius) {
    io::imRead("./data/footprint/footprint.png", _footprintFullSize, true);

    _radius          = radius;
    const auto width = static_cast<int32_t>(2.0 * std::ceil(radius) + 1.0);
    _sizeMap         = static_cast<int32_t>(std::ceil(std::sqrt(2.0) * width));

    // resize the footprint to the radius and pad to cover all rotations.
    const auto pad = (_sizeMap - width) / 2;
    _footprint     = PaddedMat(ScaledMat(_footprintFullSize, width, width), pad,
                           pad, pad, pad, 0.0);

    _pickupMap = PaintLayer<vector_type>(_sizeMap, _sizeMap);
    _pickupMap.clear();
  }

  /**
   * @brief Imprints the canvas at a specific location with the currently set paint and state of the pickup map and
   * footprint.
   *
   * @param center anchor point of the brush in canvas coordinates
   * @param theta yaw angle of the brush
   * @param canvas the canvas to paint to
   */
  void imprint(const vec2& center, const double theta,
               Canvas<vector_type>& canvas) {
    const int32_t h  = _footprint.rows;
    const int32_t w  = _footprint.cols;
    const int32_t hr = (h - 1) / 2;
    const int32_t wr = (w - 1) / 2;

    if (_useSnapshot) {
      updateSnapshot(canvas, center);
    }
    auto& pickupSoure =
      (_useSnapshot) ? _snapshotBuffer : canvas.getPaintLayer();

    const auto now = std::chrono::system_clock::now();

    std::array<double, 3UL> meanVolumes = {0.0, 0.0, 0.0};
    auto counter                        = 0U;
    for (int32_t row = -hr; row <= hr; row++) {
      for (int32_t col = -wr; col <= wr; col++) {
        const vec<int32_t, 2UL> xy_canvas = {col + center[0U],
                                             row + center[1U]};

        const auto cosTheta            = std::cos(-theta);
        const auto sinTheta            = std::sin(-theta);
        const auto rotatedCol          = col * cosTheta - row * sinTheta;
        const auto rotatedRow          = col * sinTheta + row * cosTheta;
        const vec<int32_t, 2UL> xy_map = {std::round(rotatedCol + wr),
                                          std::round(rotatedRow + hr)};

        // skip sampels outside of canvas
        if ((xy_canvas[1U] < 0) || (xy_canvas[0U] < 0) ||
            (xy_canvas[0U] >= canvas.getPaintLayer().getCols()) ||
            (xy_canvas[1U] >= canvas.getPaintLayer().getRows())) {
          continue;
        }

        // skip samples outside of pickup map
        if ((xy_map[1U] < 0) || (xy_map[0U] < 0) || (xy_map[0U] >= _sizeMap) ||
            (xy_map[1U] >= _sizeMap)) {
          continue;
        }

        canvas.checkDry(xy_canvas[0U], xy_canvas[1U], now);

        // const auto footprintHeight = _footprint(xy_map[1U], xy_map[0U]);
        {
          counter++;

          meanVolumes[0U] +=
            pickupSoure.getV_buffer()(xy_canvas[1U], xy_canvas[0U]);

          pickupPaint(xy_canvas, xy_map, pickupSoure);
          meanVolumes[1U] +=
            pickupSoure.getV_buffer()(xy_canvas[1U], xy_canvas[0U]);

          depositPaint(xy_canvas, xy_map, canvas.getPaintLayer());
          meanVolumes[2U] +=
            canvas.getPaintLayer().getV_buffer()(xy_canvas[1U], xy_canvas[0U]);
        }
      }
    }

    for (auto& v : meanVolumes) {
      v /= static_cast<double>(counter);
    }
    // std::cout << "\nmean volume before any interaction: " << meanVolumes[0U]
    //           << std::endl;
    // std::cout << "mean volume after after pickup: " << meanVolumes[1U]
    //           << std::endl;
    // std::cout << "mean volume after deposit: " << meanVolumes[2U] << std::endl;
  }

  /**
   * @brief Dip the brush in paint paint.
   *
   * @param paint
   */
  void dip(const std::array<vector_type, 2UL>& paint) {
    clean();

    _paintIntrinsic = paint;
  }

  /**
   * @brief Clear the pickup map
   *
   */
  void clean() {
    for (auto i = 0; i < _sizeMap; i++) {
      for (auto j = 0; j < _sizeMap; j++) {
        _pickupMap.set(i, j, vector_type::Zero(), vector_type::Zero(), 0.0);
      }
    }
  }

  void updateSnapshot(const Canvas<vector_type>& canvas) {
    // update the snapshot buffer which is used for paint pickup
    // the buffer gets resized accordingly in the called function
    canvas.getPaintLayer().copyTo(_snapshotBuffer);
  }

  const PaintLayer<vector_type>& getPickupMap() const {
    return _pickupMap;
  }

  const Mat<double>& getFootprint() const {
    return _footprint;
  }

  void setPickupRate(const T rate) {
    _pickupRate = rate;
  }

  void setDepositionRate(const T rate) {
    _depositionRate = rate;
  }

  T getPickupRate() const {
    return _pickupRate;
  }

  T getDepositionRate() const {
    return _depositionRate;
  }

  bool getUseSnapshotBuffer() const {
    return _useSnapshot;
  }

  void setUseSnapshotBuffer(const bool use) {
    _useSnapshot = use;
  }

 private:
  /**
   * @brief Updates the snapshot buffer to the state of the canvas leaving out the area covered by the pickup map with
   * respect to the current brush position.
   *
   * @param canvas the canvas to copy from.
   * @param exceptCenter the center point of the brush. Anchor point.
   */
  void updateSnapshot(const Canvas<vector_type>& canvas,
                      const vec2& exceptCenter) {
    // check if snapshot buffer has the correct size
    if ((canvas.getPaintLayer().getCols() != _snapshotBuffer.getCols()) ||
        (canvas.getPaintLayer().getRows() != _snapshotBuffer.getRows())) {
      canvas.getPaintLayer().copyTo(_snapshotBuffer);
    }

    const int32_t h  = _footprint.rows;
    const int32_t w  = _footprint.cols;
    const int32_t hr = (h - 1) / 2;
    const int32_t wr = (w - 1) / 2;

    const vec<int32_t, 2UL> topLeft     = {exceptCenter[0U] - wr,
                                       exceptCenter[1U] - hr};
    const vec<int32_t, 2UL> bottomRight = {exceptCenter[0U] + wr,
                                           exceptCenter[1U] + hr};

    const auto& layer = canvas.getPaintLayer();

    const vec<int32_t, 2UL> topLeftAllowed = {
      std::max(static_cast<int32_t>(exceptCenter[0U] - wr - _radius), 0),
      std::max(static_cast<int32_t>(exceptCenter[1U] - hr - _radius), 0)};
    const vec<int32_t, 2UL> bottomRightAllowed = {
      std::min(static_cast<int32_t>(exceptCenter[0U] + wr + _radius),
               static_cast<int32_t>(layer.getCols() - 1)),
      std::min(static_cast<int32_t>(exceptCenter[1U] + hr + _radius),
               static_cast<int32_t>(layer.getRows() - 1))};

    for (auto row = topLeftAllowed[1U]; row <= bottomRightAllowed[1U]; row++) {
      for (auto col = topLeftAllowed[0U]; col <= bottomRightAllowed[0U];
           col++) {
        if ((row > topLeft[1U]) && (row < bottomRight[1U]) &&
            (col > topLeft[0U]) && (col < bottomRight[0U])) {
          continue;
        }
        _snapshotBuffer.set(row, col, layer.getK_buffer()(row, col),
                            layer.getS_buffer()(row, col),
                            layer.getV_buffer()(row, col));
      }
    }
  }

  /**
   * @brief Generic linear interpolation function.
   *
   * @tparam Type
   * @param v_a
   * @param a
   * @param v_b
   * @param b
   * @return Type
   */
  template <class Type>
  Type blend(const T v_a, const Type& a, const T v_b, const Type& b) const {
    constexpr auto Eps = static_cast<T>(0.0000001);
    const T vTotal     = v_a + v_b;

    auto res = a;
    if (vTotal > Eps) {
      res = (v_a * a + v_b * b) / vTotal;
    }
    return res;
  }

  /**
   * @brief Pickup paint at a specific position.
   *
   * @param xy_canvas canvas position
   * @param xy_map corresponding pickup and footprint position
   * @param canvasLayer the paint layer to pickup from
   */
  void pickupPaint(const vec<int32_t, 2UL>& xy_canvas,
                   const vec<int32_t, 2UL>& xy_map,
                   PaintLayer<vector_type>& canvasLayer) {
    const auto footprintHeight = _footprint(xy_map[1U], xy_map[0U]);

    // TODO consider blending only with max volume 1? restrict volume to 1 als on canvas?

    if (footprintHeight > 0.0) {
      const auto v_pickupIs = _pickupMap.getV_buffer()(xy_map[1U], xy_map[0U]);

      // pickup map
      const auto v_canvasIs =
        canvasLayer.getV_buffer()(xy_canvas[1U], xy_canvas[0U]);
      const auto v_canvasLeave = _pickupRate * v_canvasIs * footprintHeight;

      // transfer paint to pickup map from canvas
      if (v_canvasLeave > 0.0) {
        // update volume on canvas
        const auto v_canvasRemain = v_canvasIs - v_canvasLeave;
        canvasLayer.getV_buffer()(xy_canvas[1U], xy_canvas[0U]) =
          v_canvasRemain;

        // update color and volume in pickup map
        const auto k =
          blend(v_pickupIs, _pickupMap.getK_buffer()(xy_map[1U], xy_map[0U]),
                v_canvasLeave,
                canvasLayer.getK_buffer()(xy_canvas[1U], xy_canvas[0U]));
        const auto s =
          blend(v_pickupIs, _pickupMap.getS_buffer()(xy_map[1U], xy_map[0U]),
                v_canvasLeave,
                canvasLayer.getS_buffer()(xy_canvas[1U], xy_canvas[0U]));
        _pickupMap.set(xy_map[1U], xy_map[0U], k, s,
                       v_pickupIs + v_canvasLeave);
      }
    }
  }

  /**
   * @brief Distribute paint at a specific position.
   *
   * @param xy_canvas canvas position
   * @param xy_map corresponding pickup and footprint position
   * @param canvasLayer the canvas to distibute paint to
   */
  void depositPaint(const vec<int32_t, 2UL>& xy_canvas,
                    const vec<int32_t, 2UL>& xy_map,
                    PaintLayer<vector_type>& canvasLayer) {
    const auto footprintHeight = _footprint(xy_map[1U], xy_map[0U]);

    if (footprintHeight > 0.0) {
      // compute blend color from pickup map and brush color
      vector_type k_source = vector_type::Zero();
      vector_type s_source = vector_type::Zero();

      const auto v_pickupIs = _pickupMap.getV_buffer()(xy_map[1U], xy_map[0U]);

      // if the pickup map is quite empty, blend with brush color
      const auto v_pickupFree =
        std::max(0.0, _pickupMapMaxCapacity - v_pickupIs);
      k_source =
        blend(v_pickupIs, _pickupMap.getK_buffer()(xy_map[1U], xy_map[0U]),
              v_pickupFree, _paintIntrinsic[0U]);
      s_source =
        blend(v_pickupIs, _pickupMap.getS_buffer()(xy_map[1U], xy_map[0U]),
              v_pickupFree, _paintIntrinsic[1U]);

      const auto v_pickupLeave = _depositionRate * v_pickupIs * footprintHeight;
      const auto v_pickupRemain = v_pickupIs - v_pickupLeave;
      _pickupMap.getV_buffer()(xy_map[1U], xy_map[0U]) = v_pickupRemain;

      const auto v_canvasIs =
        canvasLayer.getV_buffer()(xy_canvas[1U], xy_canvas[0U]);

      const auto v_Blend = _pickupMapMaxCapacity * footprintHeight;
      const auto k =
        blend(v_Blend, k_source, v_canvasIs,
              canvasLayer.getK_buffer()(xy_canvas[1U], xy_canvas[0U]));
      const auto s =
        blend(v_Blend, s_source, v_canvasIs,
              canvasLayer.getS_buffer()(xy_canvas[1U], xy_canvas[0U]));
      canvasLayer.set(xy_canvas[1U], xy_canvas[0U], k, s, v_Blend + v_canvasIs);
    }
  }

  /**
   * @brief Radius of the brush.
   *
   */
  double _radius = 0.0;

  /**
   * @brief Size of footprint and pickup map based on radius. Wide enough to cover all rotations of the footprint.
   *
   */
  int32_t _sizeMap = 0;

  /**
   * @brief Height map resulting from a 3d brush footprinting.
   *
   */
  Mat<double> _footprint;

  /**
   * @brief Original image of the footprint used for scaling according to radius. ONly used when footprint is derived
   * from a 2d sample.
   *
   */
  Mat<double> _footprintFullSize;

  /**
   * @brief Paint layer storing paint picked up from the canvas during imprinting.
   *
   */
  PaintLayer<vector_type> _pickupMap;

  /**
   * @brief Buffer for picking up paint. This can be used instead of the canvas directly to avoid oversampling and quick
   * saturation of the pickup map with recently deposited paint.
   *
   * Chu et al. - Detail-Preserving Paint Modeling for 3D Brushes - NPAR 2010
   *
   */
  PaintLayer<vector_type> _snapshotBuffer;

  /**
   * @brief Whether to use the snapshot buffer or directly pickuo from the canvas.
   *
   */
  bool _useSnapshot = false;

  /**
   * @brief Max capacity of the pickup map.
   *
   */
  T _pickupMapMaxCapacity = static_cast<T>(1.0);

  /**
   * @brief Controls how fast/much paint is picked up.
   *
   */
  T _pickupRate = static_cast<T>(0.9);

  /**
   * @brief Controls how fast/much paint is distributed.
   *
   */
  T _depositionRate = static_cast<T>(0.05);

  /**
   * @brief Current brush color. [K, S]
   *
   */
  std::array<vector_type, 2UL> _paintIntrinsic;
};
}  // namespace painty
