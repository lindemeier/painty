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
      _paintLayer(size),
      _r0_substrate(size),
      _r0_substrate_copy_buffer(size) {
  clear();
}

auto painty::CanvasGpu::getSize() const -> const Size& {
  return _size;
}

void painty::CanvasGpu::clear() {
  _paintLayer.clear();
  _r0_substrate.setTo({1.0, 1.0, 1.0, 1.0});
  _r0_substrate_copy_buffer.setTo({1.0, 1.0, 1.0, 1.0});
}

auto painty::CanvasGpu::getPaintLayer() -> PaintLayerGpu& {
  return _paintLayer;
}

auto painty::CanvasGpu::getComposed() -> const GpuMat<vec4f>& {
  _r0_substrate.getTexture()->copyTo(*_r0_substrate_copy_buffer.getTexture());
  _paintLayer.composeOnto(_r0_substrate_copy_buffer);
  return _r0_substrate_copy_buffer;
}

auto painty::CanvasGpu::getCompositionLinearRgb() -> Mat3d {
  auto composed = getComposed();

  composed.download();

  const auto r0 = composed.getMat();

  Mat3d rgb(r0.size());
  for (auto i = 0; i < static_cast<int32_t>(r0.total()); i++) {
    rgb(i) = {static_cast<double>(r0(i)[0U]), static_cast<double>(r0(i)[1U]),
              static_cast<double>(r0(i)[2U])};
  }
  return rgb;
}
