/**
 * @file GpuTaskQueue.hxx
 * @author thomas lindemeier
 * @brief
 * @date 2020-10-22
 *
 */
#pragma once

#include "painty/core/ThreadPool.hxx"
#include "painty/core/Types.hxx"
#include "prgl/Window.hxx"

namespace painty {
/**
  * @brief A job queue with an initialized opengl context in a separate thread.
  *
  */
class GpuTaskQueue final {
 public:
  GpuTaskQueue(const Size& renderWindowSize);

  /**
   * @brief Add work to the queue.
   *
   * @param f The function to execute.
   * @param args The arguments for that function.
   *
   * @return std::future<typename std::result_of<Function(Args...)>::type> A future holding the result of the task.
   */
  template <class Function, class... Args>
  std::future<typename std::result_of<Function(Args...)>::type> add_task(
    Function&& f, Args&&... args) {
    return _threadedJobQueue.add_back(f, args...);
  }

  auto getWindow() const -> const prgl::Window&;
  auto getWindow() -> prgl::Window&;

 private:
  /**
   * @brief Single threaded thread pool with thread safe queue.
   *
   */
  ThreadPool _threadedJobQueue = {1U};

  /**
   * @brief The opengl window.
   *
   */
  std::unique_ptr<prgl::Window> _windowPtr = nullptr;
};
}  // namespace painty
