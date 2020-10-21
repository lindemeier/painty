/**
 * @file SbrRenderThread.cxx
 * @author thomas lindemeier
 * @brief
 * @date 2020-10-21
 *
 */
#include "painty/renderer/SbrRenderThread.hxx"

#include <iostream>

painty::SbrRenderThread::SbrRenderThread(const Size& canvasSize)
    : _windowPtr(nullptr),
      _brushPtr(nullptr),
      _canvasPtr(nullptr),
      _canvasSize(canvasSize),
      _timerWindowUpdate(),
      _timerDryStep(),
      _jobQueue(ThreadCount) {
  // initialize the gl window in the render thread
  _jobQueue
    .add_back([=]() {
      constexpr std::array<int32_t, 4UL> rgbaBits = {8, 8, 8, 8};
      _windowPtr = std::make_unique<prgl::Window>(
        _canvasSize.width, _canvasSize.height, "SbrRenderThread - Window",
        rgbaBits, 0, 0, 8, false);

      _brushPtr  = std::make_unique<TextureBrushGpu>();

      _canvasPtr = std::make_unique<CanvasGpu>(_canvasSize);
    })
    .wait();

  _timerWindowUpdate.start(std::chrono::milliseconds(1000U), [this]() {
    _jobQueue.add_back([this]() {
      _canvasPtr->getComposed().getTexture()->render(
        0.0F, 0.0F, static_cast<float>(_windowPtr->getWidth()),
        static_cast<float>(_windowPtr->getHeight()), true);
      _windowPtr->update(false);
    });
  });

  _timerDryStep.start(std::chrono::milliseconds(10000U), [this]() {
    _jobQueue.add_back([]() {
      std::cout << "drying step" << std::endl;
      // _canvasPtr->dryStep();
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
