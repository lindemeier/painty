/**
 * @file SbrRenderThread.cxx
 * @author thomas lindemeier
 * @brief
 * @date 2020-10-21
 *
 */
#include "painty/renderer/SbrRenderThread.hxx"

painty::SbrRenderThread::SbrRenderThread(
  const std::shared_ptr<GpuTaskQueue>& gpuTaskQueue, const Size& canvasSize)
    :       _brushPtr(nullptr),
      _canvasPtr(nullptr),
      _canvasSize(canvasSize),
      _gpuTaskQueue(gpuTaskQueue) {

  _brushPtr = _gpuTaskQueue
                ->add_task([]() -> std::unique_ptr<TextureBrushGpu> {
                  return std::make_unique<TextureBrushGpu>();
                })
                .get();

  _canvasPtr = _gpuTaskQueue
                 ->add_task([canvasSize]() -> std::unique_ptr<CanvasGpu> {
                   return std::make_unique<CanvasGpu>(canvasSize);
                 })
                 .get();

  _timerWindowUpdate.start(std::chrono::milliseconds(500U), [this]() {
    _gpuTaskQueue->add_task([this]() {
      _canvasPtr->getComposed().getTexture()->render(
        0.0F, 0.0F, static_cast<float>(_gpuTaskQueue->getWindow().getWidth()),
        static_cast<float>(_gpuTaskQueue->getWindow().getHeight()), true);
      _gpuTaskQueue->getWindow().update(false);
    });
  });

  _timerDryStep.start(std::chrono::milliseconds(250U), [this]() {
    _gpuTaskQueue->add_task([this]() {
      _canvasPtr->dryStep();
    });
  });
}

painty::SbrRenderThread::~SbrRenderThread() {
  _timerDryStep.stop();
  _timerWindowUpdate.stop();

  _gpuTaskQueue
    ->add_task([this]() {
      _brushPtr  = nullptr;
      _canvasPtr = nullptr;
    })
    .wait();
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
  return _gpuTaskQueue->add_task([path, radius, ks, this]() {
    _brushPtr->dip(ks);
    _brushPtr->setRadius(radius);
    _brushPtr->paintStroke(path, *_canvasPtr);
  });
}

auto painty::SbrRenderThread::getLinearRgbImage() -> std::future<Mat3d> {
  auto future = _gpuTaskQueue->add_task([this]() -> Mat3d {
    return _canvasPtr->getCompositionLinearRgb();
  });
  return future;
}

void painty::SbrRenderThread::setBrushThicknessScale(const double scale) {
  _thicknessScale = scale;
  _gpuTaskQueue->add_task([this, scale]() {
    _brushPtr->setThicknessScale(scale);
  });
}

void painty::SbrRenderThread::enableSmudge(bool enable) {
  _gpuTaskQueue->add_task([this, enable]() {
    _brushPtr->enableSmudge(enable);
  });
}

auto painty::SbrRenderThread::dryCanvas() -> std::future<void> {
  return _gpuTaskQueue->add_task([this]() {
    _canvasPtr->dryStep(1.0F);
  });
}
