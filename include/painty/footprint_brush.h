/**
 * @file footprint_brush.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-29
 *
 */
#ifndef PAINTY_FOOTPRINT_BRUSH_H
#define PAINTY_FOOTPRINT_BRUSH_H

#include <random>

#include <painty/canvas.h>
#include <painty/image_io.h>
#include <painty/paint_layer.h>

#include <iostream>

#define PRINT(x) std::cout << #x ":\t" << x << std::endl

namespace painty
{
template <class vector_type>
class FootprintBrush final
{
  using T = typename DataType<vector_type>::channel_type;
  static constexpr auto N = DataType<vector_type>::dim;

public:
  FootprintBrush() = default;

  // size has to cover all brush transfomations
  // consider padding the pickup map
  FootprintBrush(const double radius) : _sizeMap(0U), _footprint(0U, 0U), _pickupMap(0U, 0U), _snapshotBuffer(0U, 0U)

  {
    io::imRead("/home/tsl/development/painty/data/footprint/footprint.png", _footprintFullSize);

    setRadius(radius);
  }

  ~FootprintBrush() = default;

  void setRadius(const double radius)
  {
    _radius = radius;
    const auto width = static_cast<uint32_t>(2.0 * std::ceil(radius) + 1.0);
    _sizeMap = static_cast<uint32_t>(std::ceil(std::sqrt(2.0) * width));

    const auto pad = (_sizeMap - width) / 2;

    _footprint = _footprintFullSize.scaled(width, width).padded(pad, pad, pad, pad, 0.0);

    _pickupMap = PaintLayer<vector_type>(_sizeMap, _sizeMap);
    _pickupMap.clear();

    // PRINT(_radius);
    // PRINT(width);
    // PRINT(_sizeMap);

    // const auto fcols = _footprint.getCols();
    // const auto frows = _footprint.getRows();
    // PRINT(fcols);
    // PRINT(frows);
  }

  /**
   * @brief Dip the brush in paint paint.
   *
   * @param paint
   */
  void dip(const std::array<vector_type, 2UL>& paint)
  {
    clean();

    _paintIntrinsic = paint;
  }

  void clean()
  {
    for (auto i = 0U; i < _sizeMap; i++)
    {
      for (auto j = 0U; j < _sizeMap; j++)
      {
        _pickupMap.set(i, j, vector_type::Zero(), vector_type::Zero(), 0.0);
      }
    }
  }

  void updateSnapshot(const Canvas<vector_type>& canvas)
  {
    // update the snapshot buffer which is used for paint pickup
    // the buffer gets resized accordingly in the called function
    canvas.getPaintLayer().copyTo(_snapshotBuffer);
  }

  void applyTo(const vec2& center, const double theta, Canvas<vector_type>& canvas)
  {
    constexpr auto Eps = 0.0000001;
    constexpr auto time = 1.0;
    const int32_t h = _footprint.getRows();
    const int32_t w = _footprint.getCols();
    const int32_t hr = (h - 1) / 2;
    const int32_t wr = (w - 1) / 2;

    if (_useSnapshot)
    {
      updateSnapshot(canvas, center);
    }
    auto& pickupSoure = (_useSnapshot) ? _snapshotBuffer : canvas.getPaintLayer();

    const auto now = std::chrono::system_clock::now();

    std::array<double, 3UL> meanVolumes = { 0.0, 0.0, 0.0 };
    auto counter = 0U;
    for (int32_t row = -hr; row <= hr; row++)
    {
      for (int32_t col = -wr; col <= wr; col++)
      {
        const vec<int32_t, 2UL> xy_canvas = { col + center[0U], row + center[1U] };

        const auto cosTheta = std::cos(-theta);
        const auto sinTheta = std::sin(-theta);
        const auto rotatedCol = col * cosTheta - row * sinTheta;
        const auto rotatedRow = col * sinTheta + row * cosTheta;
        const vec<int32_t, 2UL> xy_map = { std::round(rotatedCol + wr), std::round(rotatedRow + hr) };

        // skip sampels outside of canvas
        if ((xy_canvas[1U] < 0) || (xy_canvas[0U] < 0) || (xy_canvas[0U] >= canvas.getPaintLayer().getCols()) ||
            (xy_canvas[1U] >= canvas.getPaintLayer().getRows()))
        {
          continue;
        }

        // skip samples outside of pickup map
        if ((xy_map[1U] < 0) || (xy_map[0U] < 0) || (xy_map[0U] >= _sizeMap) || (xy_map[1U] >= _sizeMap))
        {
          continue;
        }

        // TODO
        canvas.checkDry(xy_canvas[0U], xy_canvas[1U], now);

        const auto footprintHeight = _footprint(xy_map[1U], xy_map[0U]);
        {
          counter++;

          meanVolumes[0U] += pickupSoure.getV_buffer()(xy_canvas[1U], xy_canvas[0U]);

          pickupPaint(xy_canvas, xy_map, pickupSoure);
          meanVolumes[1U] += pickupSoure.getV_buffer()(xy_canvas[1U], xy_canvas[0U]);

          depositPaint(xy_canvas, xy_map, canvas.getPaintLayer());
          meanVolumes[2U] += canvas.getPaintLayer().getV_buffer()(xy_canvas[1U], xy_canvas[0U]);
        }
      }
    }

    for (auto& v : meanVolumes)
    {
      v /= static_cast<double>(counter);
    }
    std::cout << "\nmean volume before any interaction: " << meanVolumes[0U] << std::endl;
    std::cout << "mean volume after after pickup: " << meanVolumes[1U] << std::endl;
    std::cout << "mean volume after deposit: " << meanVolumes[2U] << std::endl;
  }

  const PaintLayer<vector_type>& getPickupMap() const
  {
    return _pickupMap;
  }

  const Mat<double>& getFootprint() const
  {
    return _footprint;
  }

private:
  void updateSnapshot(const Canvas<vector_type>& canvas, const vec2 exceptCenter)
  {
    // check if snapshot buffer has the correct size
    if ((canvas.getPaintLayer().getCols() != _snapshotBuffer.getCols()) ||
        (canvas.getPaintLayer().getRows() != _snapshotBuffer.getRows()))
    {
      canvas.getPaintLayer().copyTo(_snapshotBuffer);
    }

    const int32_t h = _footprint.getRows();
    const int32_t w = _footprint.getCols();
    const int32_t hr = (h - 1) / 2;
    const int32_t wr = (w - 1) / 2;

    const vec<int32_t, 2UL> topLeft = { exceptCenter[0U] - wr, exceptCenter[1U] - hr };
    const vec<int32_t, 2UL> bottomRight = { exceptCenter[0U] + wr, exceptCenter[1U] + hr };

    const auto& layer = canvas.getPaintLayer();

    const vec<int32_t, 2UL> topLeftAllowed = { std::max(static_cast<int32_t>(exceptCenter[0U] - wr - _radius), 0),
                                               std::max(static_cast<int32_t>(exceptCenter[1U] - hr - _radius), 0) };
    const vec<int32_t, 2UL> bottomRightAllowed = {
      std::min(static_cast<int32_t>(exceptCenter[0U] + wr + _radius), static_cast<int32_t>(layer.getCols() - 1)),
      std::min(static_cast<int32_t>(exceptCenter[1U] + hr + _radius), static_cast<int32_t>(layer.getRows() - 1))
    };

    for (auto row = topLeftAllowed[1U]; row <= bottomRightAllowed[1U]; row++)
    {
      for (auto col = topLeftAllowed[0U]; col <= bottomRightAllowed[0U]; col++)
      {
        if ((row > topLeft[1U]) && (row < bottomRight[1U]) && (col > topLeft[0U]) && (col < bottomRight[0U]))
        {
          continue;
        }
        _snapshotBuffer.set(row, col, layer.getK_buffer()(row, col), layer.getS_buffer()(row, col),
                            layer.getV_buffer()(row, col));
      }
    }
  }

  template <class Type>
  Type blend(const T v_a, const Type& a, const T v_b, const Type& b) const
  {
    constexpr auto Eps = static_cast<T>(0.0000001);
    const T vTotal = v_a + v_b;

    auto res = a;
    if (vTotal > Eps)
    {
      res = (v_a * a + v_b * b) / vTotal;
    }
    return res;
  }

  void pickupPaint(const vec<int32_t, 2UL>& xy_canvas, const vec<int32_t, 2UL>& xy_map,
                   PaintLayer<vector_type>& canvasLayer)
  {
    constexpr auto Eps = 0.0000001;

    // todo let time pass
    constexpr auto timePassed = 1.0;

    const auto footprintHeight = _footprint(xy_map[1U], xy_map[0U]);

    // TODO consider blending only with max volume 1? restrict volume to 1 als on canvas?

    if (footprintHeight > 0.0)
    {
      const auto v_pickupIs = _pickupMap.getV_buffer()(xy_map[1U], xy_map[0U]);
      const auto v_pickupFree = _pickupMapMaxCapacity - v_pickupIs;

      // pickup map
      const auto v_canvasIs = canvasLayer.getV_buffer()(xy_canvas[1U], xy_canvas[0U]);
      const auto v_canvasLeave = std::min(v_pickupFree, _transferRatePickupFromCanvas * footprintHeight * v_canvasIs *
                                                            timePassed);  // leave should be computed as shoveling
                                                                          // all volume above brush bristle height

      // transfer paint to pickup map from canvas
      if (v_canvasLeave > Eps)
      {
        // update volume on canvas
        const auto v_canvasRemain = v_canvasIs - v_canvasLeave;
        canvasLayer.getV_buffer()(xy_canvas[1U], xy_canvas[0U]) = v_canvasRemain;

        // update color and voulme in pickup map
        const auto k = blend(v_pickupIs, _pickupMap.getK_buffer()(xy_map[1U], xy_map[0U]), v_canvasLeave,
                             canvasLayer.getK_buffer()(xy_canvas[1U], xy_canvas[0U]));
        const auto s = blend(v_pickupIs, _pickupMap.getS_buffer()(xy_map[1U], xy_map[0U]), v_canvasLeave,
                             canvasLayer.getS_buffer()(xy_canvas[1U], xy_canvas[0U]));
        _pickupMap.set(xy_map[1U], xy_map[0U], k, s, v_pickupIs + v_canvasLeave);

        // PRINT(k.transpose());
        // PRINT(s.transpose());
        // PRINT(v_pickupIs);
        // PRINT(v_canvasLeave);
        // std::cout << std::endl;
      }
    }
  }

  void depositPaint(const vec<int32_t, 2UL>& xy_canvas, const vec<int32_t, 2UL>& xy_map,
                    PaintLayer<vector_type>& canvasLayer)
  {
    // todo let time pass
    constexpr auto timePassed = 1.0;

    const auto footprintHeight = _footprint(xy_map[1U], xy_map[0U]);

    // compute blend color from pickup map and brush color
    vector_type k_source = vector_type::Zero();
    vector_type s_source = vector_type::Zero();
    {
      const auto v_pickupIs = _pickupMap.getV_buffer()(xy_map[1U], xy_map[0U]);
      const auto v_pickupFree = _pickupMapMaxCapacity - v_pickupIs;
      k_source = blend(v_pickupIs, _pickupMap.getK_buffer()(xy_map[1U], xy_map[0U]), v_pickupFree, _paintIntrinsic[0U]);
      s_source = blend(v_pickupIs, _pickupMap.getS_buffer()(xy_map[1U], xy_map[0U]), v_pickupFree, _paintIntrinsic[1U]);

      const auto v_pickupLeave = _transferRatePickupMapToCanvas * footprintHeight * v_pickupIs * timePassed;
      const auto v_pickupRemain = v_pickupIs - v_pickupLeave;
      _pickupMap.getV_buffer()(xy_map[1U], xy_map[0U]) = v_pickupRemain;
    }

    const auto v_canvasIs = canvasLayer.getV_buffer()(xy_canvas[1U], xy_canvas[0U]);
    const auto v_tranferredToCanvas = footprintHeight * _pickupMapMaxCapacity * timePassed;

    const auto k =
        blend(v_tranferredToCanvas, k_source, v_canvasIs, canvasLayer.getK_buffer()(xy_canvas[1U], xy_canvas[0U]));
    const auto s =
        blend(v_tranferredToCanvas, s_source, v_canvasIs, canvasLayer.getS_buffer()(xy_canvas[1U], xy_canvas[0U]));
    canvasLayer.set(xy_canvas[1U], xy_canvas[0U], k, s, v_tranferredToCanvas + v_canvasIs);
  }

  double _radius;

  uint32_t _sizeMap;

  Mat<double> _footprint;

  Mat<double> _footprintFullSize;

  PaintLayer<vector_type> _pickupMap;

  PaintLayer<vector_type> _snapshotBuffer;

  bool _useSnapshot = true;

  T _pickupMapMaxCapacity = static_cast<T>(1.0);

  T _transferRatePickupFromCanvas = static_cast<T>(0.2);

  T _transferRatePickupMapToCanvas = static_cast<T>(0.1);

  double _currentAngle = 0.0;

  std::array<vector_type, 2UL> _paintIntrinsic;
};
}  // namespace painty

#endif  // PAINTY_FOOTPRINT_BRUSH_H
