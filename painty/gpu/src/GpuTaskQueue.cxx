/**
 * @file GpuTaskQueue.cxx
 * @author thomas lindemeier
 * @brief
 * @date 2020-10-22
 *
 */
#include "painty/gpu/GpuTaskQueue.hxx"

painty::GpuTaskQueue::GpuTaskQueue(const Size& renderWindowSize) {
  _windowPtr =
    _threadedJobQueue
      .add_back([renderWindowSize]() -> std::unique_ptr<prgl::Window> {
        constexpr std::array<int32_t, 4UL> rgbaBits = {8, 8, 8, 8};
        return std::make_unique<prgl::Window>(
          renderWindowSize.width, renderWindowSize.height,
          "GpuTaskQueue - Window", rgbaBits, 0, 0, 8, false);
      })
      .get();
}

painty::GpuTaskQueue::~GpuTaskQueue() {
  _threadedJobQueue
    .add_back([this]() {
      _windowPtr = nullptr;
    })
    .wait();
}

auto painty::GpuTaskQueue::getWindow() const -> const prgl::Window& {
  return *_windowPtr;
}

auto painty::GpuTaskQueue::getWindow() -> prgl::Window& {
  return *_windowPtr;
}
