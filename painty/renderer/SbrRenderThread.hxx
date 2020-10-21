/**
 * @file SbrRenderThread.hxx
 * @author thomas lindemeier
 * @brief
 * @date 2020-10-21
 *
 */
#pragma once

#include "painty/core/ThreadPool.hxx"
#include "painty/core/Timer.hxx"
#include "painty/core/Types.hxx"
#include "painty/renderer/CanvasGpu.hxx"
#include "painty/renderer/TextureBrushGpu.hxx"
#include "prgl/Window.hxx"

namespace painty {

class SbrRenderThread final {
 public:
  SbrRenderThread(const Size& canvasSize);

  SbrRenderThread(const SbrRenderThread&) = delete;
  SbrRenderThread& operator=(const SbrRenderThread&) = delete;

  /**
   * @brief Get the Size object
   *
   * @return Size
   */
  auto getSize() const -> Size;

  /**
   * @brief Get the Thickness Scale object
   *
   * @return double
   */
  auto getBrushThicknessScale() const -> double;

  /**
   * @brief Renders a brush stroke to the canvas in the rendering thread.
   *
   * @param path the control points of the brush stroke.
   * @param radius the radius of the brush stroke.
   * @param ks the color as K and S.
   * @return std::future<void> the future that can be used for waiting the renderer to finish.
   */
  auto render(const std::vector<vec2>& path, const double radius,
              const std::array<vec3, 2UL>& ks) -> std::future<void>;

  /**
   * @brief Get the current image of the canvas as linear rgb.
   *
   * @return std::future<Mat3d>
   */
  auto getLinearRgbImage() -> std::future<Mat3d>;

  /**
   * @brief Multiplies the brush texture height with given scale.
   *
   * @param scale
   */
  void setBrushThicknessScale(const double scale);

  /**
   * @brief
   *
   * @param enable
   */
  void enableSmudge(bool enable);

 private:
  static constexpr auto ThreadCount = 1UL;
  /**
  * @brief A glfw window with OpenGL context.
  *
  */
  std::unique_ptr<prgl::Window> _windowPtr = nullptr;

  /**
   * @brief The brush used to apply paint to the canvas.
   *
   */
  std::unique_ptr<TextureBrushGpu> _brushPtr = nullptr;

  /**
   * @brief The canvas to render into
   *
   */
  std::unique_ptr<CanvasGpu> _canvasPtr = nullptr;

  /**
   * @brief The size of the canvas created as well as the window for now.
   *
   */
  Size _canvasSize = {};

  /**
   * @brief Timer for calling window update and current result display at a certian rate.
   *
   */
  Timer _timerWindowUpdate;

  /**
   * @brief A thread pool with one single thread for rendering.
   *
   */
  ThreadPool _jobQueue;

  double _thicknessScale = 1.0;
};
}  // namespace painty
