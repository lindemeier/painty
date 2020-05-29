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
  FootprintBrush(const uint32_t size)
    : _sizeMap(size)
    , _maxBrushVolume(800.0)
    , _footprint(0, 0)
    , _pickupMap(_sizeMap, _sizeMap)
    , _paintReservoir(_sizeMap, _sizeMap)

  {
    Mat<double> fp_original;
    io::imRead("/home/tsl/development/painty/data/footprint/footprint.png", fp_original);
    _footprint = fp_original.scaled(_sizeMap, _sizeMap);

    _pickupMap.clear();
    _paintReservoir.clear();
  }

  ~FootprintBrush() = default;

  /**
   * @brief Dip the brush in paint paint.
   *
   * @param paint
   */
  void dip(const std::array<vector_type, 2UL>& paint)
  {
    clean();
    for (auto i = 0U; i < _sizeMap; i++)
    {
      for (auto j = 0U; j < _sizeMap; j++)
      {
        _paintReservoir.set(i, j, paint[0U], paint[1U], _maxBrushVolume);
        if (_footprint(i, j) > 0.)
        {
          _pickupMap.set(i, j, paint[0U], paint[1U], _maxBrushVolume * _footprint(i, j));
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

        // TODO
        // canvas.checkDry(j_canvas, i_canvas, now);

        // skip sampels outside of canvas
        if ((i_canvas < 0) || (j_canvas < 0) || (j_canvas >= canvas.getPaintLayer().getCols()) ||
            (i_canvas >= canvas.getPaintLayer().getRows()))
        {
          continue;
        }

        // skip sampels outside of pickup map
        if ((i_map < 0) || (j_map < 0) || (j_map >= _sizeMap) || (i_map >= _sizeMap))
        {
          continue;
        }

        const auto Ap = _footprint(i_map, j_map);
        if (Ap > 0.0)
        {
          const auto clamp = [](double x, double min, double max) { return std::min(max, std::max(x, min)); };

          // refill interaction layer of brush
          const auto Vi_reservoir = _paintReservoir.getV_buffer()(i_map, j_map);
          const auto Vl_reservoir = clamp(1.0 - _pickupMap.getV_buffer()(i_map, j_map), 0.0,
                                          Vi_reservoir);  // volume leaving the brush reservoir
          const auto Vp = 1. - Vl_reservoir;
          const auto Vr_reservoir = Vi_reservoir - Vl_reservoir;  // volume staying on the canvas
          const auto CiK_reservoir = _paintReservoir.getK_buffer()(i_map, j_map);
          const auto CiS_reservoir = _paintReservoir.getS_buffer()(i_map, j_map);

          // reservoir to to pickup map
          if (Vl_reservoir > 0.)
          {
            _pickupMap.getK_buffer()(i_map, j_map) =
                (Vp * _pickupMap.getK_buffer()(i_map, j_map) + Vl_reservoir * CiK_reservoir);
            _pickupMap.getS_buffer()(i_map, j_map) =
                (Vp * _pickupMap.getS_buffer()(i_map, j_map) + Vl_reservoir * CiS_reservoir);
            _pickupMap.getV_buffer()(i_map, j_map) += Vl_reservoir;
            _paintReservoir.getV_buffer()(i_map, j_map) = Vr_reservoir;
          }

          // bidirectional transfer
          // canvas
          const auto Vi_canvas = canvas.getPaintLayer().getV_buffer()(i_canvas, j_canvas);
          const auto Vl_canvas = Vi_canvas * Ap * _transferRateCanvas * time;  // volume leaving the canvas
          const auto Vr_canvas = Vi_canvas - Vl_canvas;                        // volume staying on the canvas
          const auto CiK_canvas = canvas.getPaintLayer().getK_buffer()(i_canvas, j_canvas);
          const auto CiS_canvas = canvas.getPaintLayer().getS_buffer()(i_canvas, j_canvas);

          // pickup map
          const auto Vi_brush = _pickupMap.getV_buffer()(i_map, j_map);
          const auto Vl_brush = Vi_brush * Ap * _transferRateBrush * time;  // volume leaving the pickup map
          const auto Vr_brush = Vi_brush - Vl_brush;                        // volume staying on the pickup map
          const auto CiK_brush = _pickupMap.getK_buffer()(i_map, j_map);
          const auto CiS_brush = _pickupMap.getS_buffer()(i_map, j_map);

          // brush pickup map to canvas
          if ((Vr_canvas + Vl_brush) > 0.0)
          {
            canvas.getPaintLayer().getK_buffer()(i_canvas, j_canvas) =
                (Vr_canvas * CiK_canvas + Vl_brush * CiK_brush) / (Vr_canvas + Vl_brush);
            canvas.getPaintLayer().getS_buffer()(i_canvas, j_canvas) =
                (Vr_canvas * CiS_canvas + Vl_brush * CiS_brush) / (Vr_canvas + Vl_brush);
            canvas.getPaintLayer().getV_buffer()(i_canvas, j_canvas) = Vr_canvas + Vl_brush;
          }

          // canvas to brush pickup
          if ((Vr_brush + Vl_canvas) > 0.0)
          {
            _pickupMap.getK_buffer()(i_map, j_map) =
                (Vr_brush * CiK_brush + Vl_canvas * CiK_canvas) / (Vr_brush + Vl_canvas);
            _pickupMap.getS_buffer()(i_map, j_map) =
                (Vr_brush * CiS_brush + Vl_canvas * CiS_canvas) / (Vr_brush + Vl_canvas);
            _pickupMap.getV_buffer()(i_map, j_map) = Vr_brush + Vl_canvas;
          }
        }
      }
    }
  }

private:
  uint32_t _sizeMap;

  Mat<double> _footprint;

  PaintLayer<vector_type> _pickupMap;

  PaintLayer<vector_type> _paintReservoir;

  T _maxBrushVolume = static_cast<T>(0.0);

  T _transferRateCanvas = static_cast<T>(0.1);
  T _transferRateBrush = static_cast<T>(0.2);

  double _currentAngle = 0.0;
};
}  // namespace painty

#endif  // PAINTY_FOOTPRINT_BRUSH_H
