/**
 * @file SbrRenderThread.cxx
 * @author thomas lindemeier
 * @brief
 * @date 2020-10-21
 *
 */
#include "painty/renderer/SbrRenderThread.hxx"

painty::SbrRenderThread::SbrRenderThread(const Size& canvasSize)
    : _windowPtr(nullptr),
      _brushPtr(nullptr),
      _canvasPtr(nullptr),
      _canvasSize(canvasSize),
      _timerWindowUpdate(),
      _timerDryStep(),
      _jobQueue(ThreadCount) {
  // initialize the gl window in the render thread
  _windowPtr = _jobQueue
                 .add_back([canvasSize]() -> std::unique_ptr<prgl::Window> {
                   constexpr std::array<int32_t, 4UL> rgbaBits = {8, 8, 8, 8};
                   return std::make_unique<prgl::Window>(
                     canvasSize.width, canvasSize.height,
                     "SbrRenderThread - Window", rgbaBits, 0, 0, 8, false);
                 })
                 .get();

  _brushPtr = _jobQueue
                .add_back([]() -> std::unique_ptr<TextureBrushGpu> {
                  return std::make_unique<TextureBrushGpu>();
                })
                .get();

  _canvasPtr = _jobQueue
                 .add_back([canvasSize]() -> std::unique_ptr<CanvasGpu> {
                   return std::make_unique<CanvasGpu>(canvasSize);
                 })
                 .get();

  _timerWindowUpdate.start(std::chrono::milliseconds(500U), [this]() {
    _jobQueue.add_back([this]() {
      _canvasPtr->getComposed().getTexture()->render(
        0.0F, 0.0F, static_cast<float>(_windowPtr->getWidth()),
        static_cast<float>(_windowPtr->getHeight()), true);
      _windowPtr->update(false);
    });
  });

  _timerDryStep.start(std::chrono::seconds(1U), [this]() {
    _jobQueue.add_back([this]() {
      _canvasPtr->dryStep();
    });
  });
}

auto painty::SbrRenderThread::getSize() const -> Size {
  return _canvasSize;
}

auto painty::SbrRenderThread::getBrushThicknessScale() const -> double {
  return _thicknessScale;
}

auto painty::SbrRenderThread::render(const std::vector<vec2>& path,
                                     const double radius,
                                     const std::array<vec3, 2UL>& ks)
  -> std::future<void> {
  return _jobQueue.add_back([path, radius, ks, this]() {
    _brushPtr->dip(ks);
    _brushPtr->setRadius(radius);
    _brushPtr->paintStroke(path, *_canvasPtr);
  });
}

auto painty::SbrRenderThread::getLinearRgbImage() -> std::future<Mat3d> {
  auto future = _jobQueue.add_back([this]() -> Mat3d {
    return _canvasPtr->getCompositionLinearRgb();
  });
  return future;
}

void painty::SbrRenderThread::setBrushThicknessScale(const double scale) {
  _thicknessScale = scale;
  _jobQueue.add_back([this, scale]() {
    _brushPtr->setThicknessScale(scale);
  });
}

void painty::SbrRenderThread::enableSmudge(bool enable) {
  _jobQueue.add_back([this, enable]() {
    _brushPtr->enableSmudge(enable);
  });
}
