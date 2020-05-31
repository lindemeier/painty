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
  FootprintBrush(const double radius)
    : _sizeMap(0U), _maxBrushVolume(300.0), _footprint(0U, 0U), _pickupMap(0U, 0U), _paintReservoir(0U, 0U)

  {
    io::imRead("/home/tsl/development/painty/data/footprint/footprint.png", _footprintFullSize);

    setRadius(radius);
  }

  ~FootprintBrush() = default;

  void setRadius(const double radius)
  {
    _radius = radius;

    _sizeMap = static_cast<uint32_t>(2.0 * std::ceil(radius) + 1.0);

    _footprint = _footprintFullSize.scaled(_sizeMap, _sizeMap);

    _pickupMap = PaintLayer<vector_type>(_sizeMap, _sizeMap);
    _pickupMap.clear();

    _paintReservoir = PaintLayer<vector_type>(_sizeMap, _sizeMap);
    _paintReservoir.clear();
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

    for (auto i = 0U; i < _sizeMap; i++)
    {
      for (auto j = 0U; j < _sizeMap; j++)
      {
        _paintReservoir.set(i, j, paint[0U], paint[1U], _maxBrushVolume);
        if (_footprint(i, j) > 0.)
        {
          _pickupMap.set(i, j, paint[0U], paint[1U], 0.0);
        }
      }
    }
  }

  void clean()
  {
    for (auto i = 0U; i < _sizeMap; i++)
    {
      for (auto j = 0U; j < _sizeMap; j++)
      {
        _pickupMap.set(i, j, vector_type::Zero(), vector_type::Zero(), 0.);
        _paintReservoir.set(i, j, vector_type::Zero(), vector_type::Zero(), 0.);
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

        // compute rotated coordinates
        const vec2 xy = { static_cast<double>(col) * std::cos(theta) - static_cast<double>(row) * std::sin(theta),
                          static_cast<double>(col) * std::sin(theta) + static_cast<double>(row) * std::cos(theta) };
        // nearest neighbor
        const auto i_canvas = static_cast<int32_t>(round(xy[1U] + center[1U]));
        const auto j_canvas = static_cast<int32_t>(round(xy[0U] + center[0U]));
        const auto i_map = static_cast<int32_t>(round(xy[1U] + hr));
        const auto j_map = static_cast<int32_t>(round(xy[0U] + wr));

        // skip sampels outside of canvas
        if ((i_canvas < 0) || (j_canvas < 0) || (j_canvas >= canvas.getPaintLayer().getCols()) ||
            (i_canvas >= canvas.getPaintLayer().getRows()))
        {
          continue;
        }

        // skip samples outside of pickup map
        if ((i_map < 0) || (j_map < 0) || (j_map >= _sizeMap) || (i_map >= _sizeMap))
        {
          continue;
        }

        // TODO
        canvas.checkDry(j_canvas, i_canvas, now);

        const auto fp = _footprint(i_map, j_map);
        if (fp > Eps)
        {
          refillPaint(i_map, j_map);

          pickupPaint(i_canvas, j_canvas, i_map, j_map, canvas.getPaintLayer());

          depositPaint(i_canvas, j_canvas, i_map, j_map, canvas.getPaintLayer());
        }
      }
    }
  }

private:
  void pickupPaint(const uint32_t i_canvas, const uint32_t j_canvas, const uint32_t i_map, const uint32_t j_map,
                   PaintLayer<vector_type>& canvasLayer)
  {
    // todo let time pass
    constexpr auto timePassed = 1.0;

    const auto fp = _footprint(i_map, j_map);

    const auto V_CanvasContained = canvasLayer.getV_buffer()(i_canvas, j_canvas);

    // volume leaving the canvas
    const auto V_CanvasLeaving = V_CanvasContained * _transferRateCanvas * timePassed * (1.0 - fp);
    const auto V_CanvasRemaining = V_CanvasContained - V_CanvasLeaving;
    // update volume on canvas
    canvasLayer.getV_buffer()(i_canvas, j_canvas) = V_CanvasRemaining;

    // transfer paint to pickup map from canvas
    const auto V_PickupMap = _pickupMap.getV_buffer()(i_map, j_map);
    constexpr auto Eps = 0.0000001;
    const auto V_total = V_PickupMap + V_CanvasLeaving;
    if (V_total > Eps)
    {
      const auto k = (V_PickupMap * _pickupMap.getK_buffer()(i_map, j_map) +
                      V_CanvasLeaving * canvasLayer.getK_buffer()(i_canvas, j_canvas)) /
                     V_total;
      const auto s = (V_PickupMap * _pickupMap.getS_buffer()(i_map, j_map) +
                      V_CanvasLeaving * canvasLayer.getS_buffer()(i_canvas, j_canvas)) /
                     V_total;
      _pickupMap.set(i_map, j_map, k, s, V_total);
    }
  }

  void depositPaint(const uint32_t i_canvas, const uint32_t j_canvas, const uint32_t i_map, const uint32_t j_map,
                    PaintLayer<vector_type>& canvasLayer)
  {
    // todo let time pass
    constexpr auto timePassed = 1.0;

    const auto fp = _footprint(i_map, j_map);

    const auto V_PickupMapContained = _pickupMap.getV_buffer()(i_map, j_map);

    // volume leaving the canvas
    const auto V_PickupMapLeaving = V_PickupMapContained * _transferRateBrush * timePassed * fp;
    const auto V_PickupMapRemaining = V_PickupMapContained - V_PickupMapLeaving;
    // update volume on pickupmap
    _pickupMap.getV_buffer()(i_map, j_map) = V_PickupMapRemaining;

    // transfer paint to canvas from pickup map
    const auto V_Canvas = canvasLayer.getV_buffer()(i_canvas, j_canvas);
    constexpr auto Eps = 0.0000001;
    const auto V_total = V_Canvas + V_PickupMapLeaving;
    if (V_total > Eps)
    {
      const auto k = (V_PickupMapLeaving * _pickupMap.getK_buffer()(i_map, j_map) +
                      V_Canvas * canvasLayer.getK_buffer()(i_canvas, j_canvas)) /
                     V_total;
      const auto s = (V_PickupMapLeaving * _pickupMap.getS_buffer()(i_map, j_map) +
                      V_Canvas * canvasLayer.getS_buffer()(i_canvas, j_canvas)) /
                     V_total;
      canvasLayer.set(i_canvas, j_canvas, k, s, V_total);
    }
  }

  void refillPaint(const uint32_t i_map, const uint32_t j_map)
  {
    // refill interaction layer of brush
    const auto Vi_reservoir = _paintReservoir.getV_buffer()(i_map, j_map);
    const auto Vl_reservoir = std::min(Vi_reservoir, std::max(1.0 - _pickupMap.getV_buffer()(i_map, j_map),
                                                              0.0));  // volume leaving the brush reservoir
    const auto Vp = 1.0 - Vl_reservoir;
    const auto Vr_reservoir = Vi_reservoir - Vl_reservoir;  // volume staying on the canvas
    const auto CiK_reservoir = _paintReservoir.getK_buffer()(i_map, j_map);
    const auto CiS_reservoir = _paintReservoir.getS_buffer()(i_map, j_map);

    // reservoir to to pickup map
    constexpr auto Eps = 0.0000001;
    if (Vl_reservoir > Eps)
    {
      const auto k = (Vp * _pickupMap.getK_buffer()(i_map, j_map) + Vl_reservoir * CiK_reservoir);
      const auto s = (Vp * _pickupMap.getS_buffer()(i_map, j_map) + Vl_reservoir * CiS_reservoir);
      const auto v = _pickupMap.getV_buffer()(i_map, j_map) + Vl_reservoir;

      _pickupMap.set(i_map, j_map, k, s, v);

      _paintReservoir.getV_buffer()(i_map, j_map) = Vr_reservoir;
    }
  }

  double _radius;

  uint32_t _sizeMap;

  Mat<double> _footprint;

  Mat<double> _footprintFullSize;

  PaintLayer<vector_type> _pickupMap;

  PaintLayer<vector_type> _paintReservoir;

  T _maxBrushVolume = static_cast<T>(0.0);

  T _transferRateCanvas = static_cast<T>(0.1);

  T _transferRateBrush = static_cast<T>(0.1);

  double _currentAngle = 0.0;

  std::array<vector_type, 2UL> _paintIntrinsic;
};
}  // namespace painty

#endif  // PAINTY_FOOTPRINT_BRUSH_H
