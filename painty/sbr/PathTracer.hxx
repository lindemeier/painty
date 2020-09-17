/**
 * @file PathTracer.hxx
 * @author Thomas Lindemeier
 * @brief
 *
 * @date 2020-09-11
 *
 */
#pragma once

#include <vector>

#include "painty/core/Vec.hxx"
#include "painty/image/Mat.hxx"

namespace painty {
class PathTracer {
  struct Stepper {
    vec2 p    = vec2::Zero();
    vec2 t    = vec2::Zero();
    double w  = 0.0;
    double dw = 0.0;
  };

 public:
  enum class NextAction : uint8_t {
    PATH_CONTINUE,
    PATH_STOP_NEXT,
    PATH_STOP_NOW
  };

  PathTracer(const Mat3d& tensor_field);

  std::vector<vec2> trace(const vec2& seed);

  void setTensorField(const Mat3d& tensors);

  uint32_t getMaxLen() const;

  void setMaxLen(uint32_t maxLen);

  uint32_t getMinLen() const;

  void setMinLen(uint32_t minLen);

  const cv::Rect2i& getFrame() const;

  void setFrame(const cv::Rect2i& frame);

  double getStep() const;

  void setStep(double step);

  double getFc() const;

  void setFc(double fc);

  void setEvaluatePositionFun(std::function<NextAction(const vec2&)> fun);

 private:
  Mat3d _tensor_field = {};

  /**
   * @brief hard constraint for maximum numbers of points in the traced path.
   *
   */
  uint32_t _maxLen = 12U;

  /**
   * @brief hard constraint for minimum numbers of points in the traced path.
   *
   */
  uint32_t _minLen = 2U;

  /**
   * @brief Bounding frame as hard constraint.
   *
   */
  cv::Rect2i _frame = {};

  /**
   * @brief Step size of each step.
   *
   */
  double _step = 1.0;

  /**
   * @brief Curvature parameter of the path tracer. 1 for following the edge tangent flow as close as possible, 0 for use the previous direction.
   *
   */
  double _fc = 1.0;

  /**
   * @brief A function that is called at every step that checks for user defined constraints.
   *
   */
  std::function<NextAction(const vec2&)> _evaluatePositionFun = {};

  /**
   * @brief Advance in the path by setting the Stepper helper struct.
   *
   * @param s the step to set
   * @return true
   * @return false
   */
  bool stepNext(Stepper& s) const;

  /**
   * @brief check whether a point lies inside of the given frame.
   *
   * @param p the point to check
   * @return true
   * @return false
   */
  bool insideFrame(const vec2& p) const;
};
}  // namespace painty
