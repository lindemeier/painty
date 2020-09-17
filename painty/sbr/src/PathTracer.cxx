/**
 * @file PathTracer.cxx
 * @author Thomas Lindemeier
 * @brief
 *
 * @date 2020-09-11
 *
 */
#include "painty/sbr/PathTracer.hxx"

#include <deque>

#include "painty/image/EdgeTangentFlow.hxx"

namespace painty {
PathTracer::PathTracer(const Mat3d& tensor_field)
    : _tensor_field(tensor_field),
      _frame(0, 0, tensor_field.cols, tensor_field.rows) {
  _evaluatePositionFun = [this](const vec2& cPos) -> NextAction {
    if (insideFrame(cPos)) {
      return NextAction::PATH_CONTINUE;
    }
    return NextAction::PATH_STOP_NOW;
  };
}

auto PathTracer::trace(const vec2& seed) -> std::vector<vec2> {
  std::vector<vec2> path;

  Stepper forward;
  Stepper backward;
  forward.p[0] = backward.p[0] = seed[0];
  forward.p[1] = backward.p[1] = seed[1];

  const auto minEv =
    tensor::GetMinEigenVector(Interpolate(_tensor_field, seed));
  vec2 t(minEv[0], minEv[1]);
  const auto m = t.norm();
  if (m > 0.0) {
    t.normalize();
  } else {
    t = {0.0, 1.0};
  }
  backward.t = forward.t = t;
  backward.t *= -1.0;
  forward.w = backward.w = 0.0;

  std::deque<vec2> pathDeq;

  auto growF = true;
  auto growB = true;

  // push first point
  if ((((forward.w + backward.w) / _step) >= _minLen) &&
      (_evaluatePositionFun(seed) != NextAction::PATH_CONTINUE)) {
    growF = growB = false;
  } else {
    pathDeq.push_back(seed);
    forward.w += _step;
  }

  while ((((forward.w + backward.w) / _step) < _maxLen) && (growF || growB)) {
    // grow forwards
    if (growF) {
      if (stepNext(forward) && insideFrame(forward.p)) {
        const auto st = _evaluatePositionFun(forward.p);
        if (st == NextAction::PATH_STOP_NOW) {
          growF = false;
        } else {
          if ((forward.w + backward.w) >= _minLen &&
              st != NextAction::PATH_CONTINUE) {
            growF = false;
          } else {
            if (pathDeq.empty() || pathDeq.back() != forward.p) {
              pathDeq.push_back(forward.p);
            }
          }
        }
      } else {
        growF = false;
      }
    }

    // grow backwards
    if (growB) {
      if (stepNext(backward) && insideFrame(backward.p)) {
        const auto st = _evaluatePositionFun(backward.p);
        if (st == NextAction::PATH_STOP_NOW) {
          growF = false;
        } else {
          if ((forward.w + backward.w) >= _minLen &&
              st != NextAction::PATH_CONTINUE) {
            growB = false;
          } else {
            if (pathDeq.empty() || pathDeq.front() != backward.p) {
              pathDeq.push_front(backward.p);
            }
          }
        }
      } else {
        growB = false;
      }
    }
  }
  path.reserve(pathDeq.size());
  path.insert(path.end(), pathDeq.cbegin(), pathDeq.cend());

  return path;
}

void PathTracer::setTensorField(const Mat3d& tensor_field) {
  _tensor_field = tensor_field;
}

uint32_t PathTracer::getMaxLen() const {
  return _maxLen;
}

void PathTracer::setMaxLen(uint32_t maxLen) {
  _maxLen = maxLen;
}

uint32_t PathTracer::getMinLen() const {
  return _minLen;
}

void PathTracer::setMinLen(uint32_t minLen) {
  _minLen = minLen;
}

const cv::Rect2i& PathTracer::getFrame() const {
  return _frame;
}

void PathTracer::setFrame(const cv::Rect2i& frame) {
  _frame = frame;
}

double PathTracer::getStep() const {
  return _step;
}

void PathTracer::setStep(double step) {
  _step = step;
}

double PathTracer::getFc() const {
  return _fc;
}

void PathTracer::setFc(double fc) {
  _fc = fc;
}

void PathTracer::setEvaluatePositionFun(
  std::function<NextAction(const vec2&)> fun) {
  _evaluatePositionFun = fun;
}

bool PathTracer::stepNext(Stepper& s) const {
  vec3 ten = Interpolate(_tensor_field, s.p);

  vec2 minEv = ::painty::tensor::GetMinEigenVector(ten);

  vec2 t(minEv[0], minEv[1]);
  double m = t.norm();

  if (m > 0.0) {
    t.normalize();
  } else {
    t = {0.0, 1.0};
  }

  vec2 t_last = s.t;

  double dot = t.dot(t_last);

  // reverse if direction points backwards
  if (dot < 0.0) {
    t *= -1.0;
  }

  // filter with previous direction
  if (_fc < 1.0) {
    double dx = t[0];
    double dy = t[1];
    dx        = _fc * dx + (1.0 - _fc) * (t_last[0]);
    dy        = _fc * dy + (1.0 - _fc) * (t_last[1]);
    t[0]      = dx / std::pow(dx * dx + dy * dy, 0.5);
    t[1]      = dy / std::pow(dx * dx + dy * dy, 0.5);
    t.normalize();
  }

  s.t[0] = t[0];
  s.t[1] = t[1];

  if (_step <= 1.0) {
    s.dw =
      (std::abs(t[0]) >= std::abs(t[1]))
        ? std::abs(((s.p[0] - std::floor(s.p[0])) - 0.5 - painty::sgn(t[0])) /
                   (t[0]))
        : std::abs(((s.p[1] - std::floor(s.p[1])) - 0.5 - painty::sgn(t[1])) /
                   (t[1]));
  } else {
    s.dw = _step;
  }

  s.p[0] += t[0] * s.dw;
  s.p[1] += t[1] * s.dw;
  s.w += s.dw;

  return true;
}

bool PathTracer::insideFrame(const vec2& p) const {
  return p[0] >= _frame.x && p[0] < _frame.x + _frame.width &&
         p[1] >= _frame.y && p[1] < _frame.y + _frame.height;
}
}  // namespace painty
