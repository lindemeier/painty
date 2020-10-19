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
      _r0_substrate(size) {
  clear();
}

auto painty::CanvasGpu::getSize() const -> const Size& {
  return _size;
}

void painty::CanvasGpu::clear() {
  _paintLayer.clear();
  _r0_substrate.setTo({1.0, 1.0, 1.0, 1.0});
}

auto painty::CanvasGpu::getPaintLayer() -> PaintLayerGpu& {
  return _paintLayer;
}

auto painty::CanvasGpu::getComposition(
  const std::shared_ptr<prgl::Window>& windowPtr) -> Mat3d {
  // set currently to white
  // After implementing drying, change this to a temporary
  _r0_substrate.setTo({1.0, 1.0, 1.0, 1.0});

  _paintLayer.composeOnto(_r0_substrate);

  _r0_substrate.getTexture()->render(
    0.0F, 0.0F, static_cast<float>(windowPtr->getWidth()),
    static_cast<float>(windowPtr->getHeight()));
  windowPtr->update(false);

  _r0_substrate.download();

  const auto r0 = _r0_substrate.getMat();

  Mat3d rgb(r0.size());
  for (auto i = 0; i < static_cast<int32_t>(r0.total()); i++) {
    rgb(i) = {static_cast<double>(r0(i)[0U]), static_cast<double>(r0(i)[1U]),
              static_cast<double>(r0(i)[2U])};
  }
  return rgb;
}
