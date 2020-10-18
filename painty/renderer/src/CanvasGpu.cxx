/**
 * @file CanvasGpu.cxx
 * @author thomas lindemeier
 * @brief
 * @date 2020-10-18
 *
 */
#include "painty/renderer/CanvasGpu.hxx"

painty::CanvasGpu::CanvasGpu(const Size& size)
    : _size(size),
      _paintLayer(size) {}

auto painty::CanvasGpu::getSize() const -> const Size& {
  return _size;
}

void painty::CanvasGpu::clear() {
  _paintLayer.clear();
}
