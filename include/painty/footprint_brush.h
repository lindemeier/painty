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
  FootprintBrush(const double radius) : _sizeMap(0U), _maxBrushVolume(300.0), _footprint(0U, 0U), _pickupMap(0U, 0U)

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

    PRINT(_radius);
    PRINT(width);
    PRINT(_sizeMap);

    const auto fcols = _footprint.getCols();
    const auto frows = _footprint.getRows();
    PRINT(fcols);
    PRINT(frows);
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
        _pickupMap.set(i, j, vector_type::Zero(), vector_type::Zero(), 0.);
      }
    }
  }

  void applyTo(const vec2& center, const double theta, Canvas<vector_type>& canvas)
  {
    constexpr auto Eps = 0.0000001;
    constexpr auto time = 1.0;
    const int32_t h = _footprint.getRows();
    const int32_t w = _footprint.getCols();
    const int32_t hr = (h - 1) / 2;
    const int32_t wr = (w - 1) / 2;

    const auto now = std::chrono::system_clock::now();

    for (int32_t row = -hr; row <= hr; row++)
    {
      for (int32_t col = -wr; col <= wr; col++)
      {
        // TODO use rotated canvas coordinates bounding rectangle (axis aligned)
        // inverse!

        const vec<int32_t, 2UL> xy_canvas = { col + center[0U], row + center[1U] };
        const vec<int32_t, 2UL> xy_map = { col + wr, row + hr };

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

        const auto fp = _footprint(xy_map[1U], xy_map[0U]);
        if (fp > Eps)
        {
          pickupPaint(xy_canvas, xy_map, canvas.getPaintLayer());

          depositPaint(xy_canvas, xy_map, canvas.getPaintLayer());
        }
      }
    }
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
    // todo let time pass
    constexpr auto timePassed = 1.0;

    const auto fp = _footprint(xy_map[1U], xy_map[0U]);
    const auto V_PickupMap = _pickupMap.getV_buffer()(xy_map[1U], xy_map[0U]);
    const auto leftInPickupMap = _pickupMapMaxCapacity - V_PickupMap;

    const auto V_CanvasContained = canvasLayer.getV_buffer()(xy_canvas[1U], xy_canvas[0U]);

    // volume leaving the canvas
    const auto V_CanvasLeaving =
        std::min(leftInPickupMap, V_CanvasContained * _transferRatePickupFromCanvas * timePassed * fp);
    const auto V_CanvasRemaining = V_CanvasContained - V_CanvasLeaving;
    // update volume on canvas
    canvasLayer.getV_buffer()(xy_canvas[1U], xy_canvas[0U]) = V_CanvasRemaining;

    // transfer paint to pickup map from canvas
    // TODO max capacity of pickup map
    constexpr auto Eps = 0.0000001;
    const auto V_total = V_PickupMap + V_CanvasLeaving;
    if (V_total > Eps)
    {
      const auto k = blend(V_PickupMap, _pickupMap.getK_buffer()(xy_map[1U], xy_map[0U]), V_CanvasLeaving,
                           canvasLayer.getK_buffer()(xy_canvas[1U], xy_canvas[0U]));

      const auto s = blend(V_PickupMap, _pickupMap.getS_buffer()(xy_map[1U], xy_map[0U]), V_CanvasLeaving,
                           canvasLayer.getS_buffer()(xy_canvas[1U], xy_canvas[0U]));
      _pickupMap.set(xy_map[1U], xy_map[0U], k, s, V_total);
    }
  }

  void depositPaint(const vec<int32_t, 2UL>& xy_canvas, const vec<int32_t, 2UL>& xy_map,
                    PaintLayer<vector_type>& canvasLayer)
  {
    // todo let time pass
    constexpr auto timePassed = 1.0;

    const auto fp = _footprint(xy_map[1U], xy_map[0U]);

    const auto V_PickupMapContained = _pickupMap.getV_buffer()(xy_map[1U], xy_map[0U]);

    // volume leaving the pickup map
    const auto V_PickupMapLeaving = V_PickupMapContained * _transferRatePickupMapToCanvas * timePassed * fp;
    const auto V_PickupMapRemaining = V_PickupMapContained - V_PickupMapLeaving;
    // update volume on pickupmap
    _pickupMap.getV_buffer()(xy_map[1U], xy_map[0U]) = V_PickupMapRemaining;

    // transfer paint to canvas from brush and pickup color
    const auto V_BrushLeave = _maxBrushVolume * _transferRatePickupMapToCanvas * timePassed *
                              fp;  // TODO figure out correct brush intrinsic load vloume
                                   // for mixing source color and blend with canvas

    constexpr auto Eps = 0.0000001;
    const auto V_total = V_BrushLeave + V_PickupMapLeaving;
    if (V_total > Eps)
    {
      // blend brush color with pickup color as source color
      const auto invPickupMapContained = _pickupMapMaxCapacity - V_PickupMapContained;
      const auto k_source = blend(V_PickupMapContained, _pickupMap.getK_buffer()(xy_map[1U], xy_map[0U]),
                                  invPickupMapContained, _paintIntrinsic[0U]);
      const auto s_source = blend(V_PickupMapContained, _pickupMap.getS_buffer()(xy_map[1U], xy_map[0U]),
                                  invPickupMapContained, _paintIntrinsic[1U]);

      // blend source color with canvas
      const auto V_canvas = canvasLayer.getV_buffer()(xy_canvas[1U], xy_canvas[0U]);
      const auto k = blend(V_canvas, canvasLayer.getK_buffer()(xy_canvas[1U], xy_canvas[0U]), V_total, k_source);
      const auto s = blend(V_canvas, canvasLayer.getS_buffer()(xy_canvas[1U], xy_canvas[0U]), V_total, s_source);

      canvasLayer.set(xy_canvas[1U], xy_canvas[0U], k, s, V_total + V_canvas);
    }
  }

  double _radius;

  uint32_t _sizeMap;

  Mat<double> _footprint;

  Mat<double> _footprintFullSize;

  T _maxBrushVolume = static_cast<T>(0.0);

  PaintLayer<vector_type> _pickupMap;

  T _pickupMapMaxCapacity = static_cast<T>(1.0);

  T _transferRatePickupFromCanvas = static_cast<T>(0.1);

  T _transferRatePickupMapToCanvas = static_cast<T>(0.1);

  double _currentAngle = 0.0;

  std::array<vector_type, 2UL> _paintIntrinsic;
};
}  // namespace painty

#endif  // PAINTY_FOOTPRINT_BRUSH_H
