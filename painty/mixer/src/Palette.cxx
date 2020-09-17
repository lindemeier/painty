/**
 * @file Palette.cxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2019-04-13
 *
 */
#include "painty/mixer/Palette.hxx"

#include <fstream>
#include <thread>

#include "ceres/ceres.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "painty/mixer/Serialization.hxx"

namespace ceres {
template <class T>
inline T coth(const T& x)  // hyperbolic cotangent
{
  const auto TOO_HIGH_X = static_cast<T>(20.0);
  if (x > TOO_HIGH_X) {
    return static_cast<T>(1.0);
  }
  if (ceres::abs(x) > static_cast<T>(0.0)) {
    return ceres::cosh(x) / ceres::sinh(x);
  }
  return std::numeric_limits<T>::infinity();
}

}  // namespace ceres

namespace MeasureSolver {
static const double d_bound_upper      = 5.;
static const double d_bound_lower      = 1e-9;
static const double measure_threshold  = 0.0005;
static const double KS_lower_bound     = 1e-9;
static const double KS_upper_bound     = 5.0;
static const double function_tolerance = 1e-9;
static const int32_t max_iterations    = 300;

struct CostFunction_KS_d_map {
  CostFunction_KS_d_map(const ::painty::vec3& R0, const ::painty::vec3& R1)
      : _R0(R0),
        _R1(R1) {}

  template <typename T>
  bool operator()(T const* const* parameters, T* residuals) const {
    const T Eps = T(1e-9);

    T Kr = parameters[0][0];
    T Kg = parameters[0][1];
    T Kb = parameters[0][2];
    T Sr = parameters[1][0];
    T Sg = parameters[1][1];
    T Sb = parameters[1][2];
    T d  = parameters[2][0];

    if ((ceres::abs(Kr) < Eps) || (ceres::abs(Kg) < Eps) ||
        (ceres::abs(Kb) < Eps) || (ceres::abs(Sr) < Eps) ||
        (ceres::abs(Sg) < Eps) || (ceres::abs(Sb) < Eps) ||
        (ceres::abs(d) < Eps)) {
      return false;
    }

    // compute reflectance
    T ar   = T(1.) + Kr / Sr;
    T ag   = T(1.) + Kg / Sg;
    T ab   = T(1.) + Kb / Sb;
    T asqr = ceres::pow(ar, T(2));
    T asqg = ceres::pow(ag, T(2));
    T asqb = ceres::pow(ab, T(2));

    if ((ceres::abs(asqr) < Eps) || (ceres::abs(asqg) < Eps) ||
        (ceres::abs(asqb) < Eps)) {
      return false;
    }

    T br   = ceres::sqrt(asqr - T(1.));
    T bg   = ceres::sqrt(asqg - T(1.));
    T bb   = ceres::sqrt(asqb - T(1.));
    T bShr = br * Sr * d;
    T bShg = bg * Sg * d;
    T bShb = bb * Sb * d;

    if ((ceres::abs(bShr) < Eps) || (ceres::abs(bShg) < Eps) ||
        ((ceres::abs(bShb) < Eps))) {
      return false;
    }

    T bcothbShr = br * ceres::coth(bShr);
    T bcothbShg = bg * ceres::coth(bShg);
    T bcothbShb = bb * ceres::coth(bShb);
    T Rr = (T(1.) - _R0[0] * (ar - bcothbShr)) / (ar - _R0[0] + bcothbShr);
    T Rg = (T(1.) - _R0[1] * (ag - bcothbShg)) / (ag - _R0[1] + bcothbShg);
    T Rb = (T(1.) - _R0[2] * (ab - bcothbShb)) / (ab - _R0[2] + bcothbShb);

    residuals[0] = Rr - _R1[0];
    residuals[1] = Rg - _R1[1];
    residuals[2] = Rb - _R1[2];

    return true;
  }

  // Factory to hide the construction of the CostFunction object from
  // the client code.
  static ::ceres::CostFunction* Create(const ::painty::vec3& R0,
                                       const ::painty::vec3& R1) {
    auto* c =
      (new ::ceres::DynamicAutoDiffCostFunction<CostFunction_KS_d_map, 4>(
        new CostFunction_KS_d_map(R0, R1)));
    c->SetNumResiduals(3);
    c->AddParameterBlock(3);
    c->AddParameterBlock(3);
    c->AddParameterBlock(1);
    return c;
  }

 private:
  ::painty::vec3 _R0;
  ::painty::vec3 _R1;
};

struct CostFunction_KS_single_d {
  CostFunction_KS_single_d(const ::painty::vec3& R0, const ::painty::vec3& R1)
      : _R0(R0),
        _R1(R1) {}

  template <typename T>
  bool operator()(const T* const K, const T* const S, const T* const thickness,
                  T* residuals) const {
    const T Eps = T(1e-9);

    T Kr = K[0];
    T Kg = K[1];
    T Kb = K[2];
    T Sr = S[0];
    T Sg = S[1];
    T Sb = S[2];
    T d  = thickness[0];

    if ((ceres::abs(Kr) < Eps) || (ceres::abs(Kg) < Eps) ||
        (ceres::abs(Kb) < Eps) || (ceres::abs(Sr) < Eps) ||
        (ceres::abs(Sg) < Eps) || (ceres::abs(Sb) < Eps) ||
        (ceres::abs(d) < Eps)) {
      return false;
    }

    // compute reflectance
    T ar   = T(1.) + Kr / Sr;
    T ag   = T(1.) + Kg / Sg;
    T ab   = T(1.) + Kb / Sb;
    T asqr = ceres::pow(ar, T(2));
    T asqg = ceres::pow(ag, T(2));
    T asqb = ceres::pow(ab, T(2));

    if ((ceres::abs(asqr) < Eps) || (ceres::abs(asqg) < Eps) ||
        (ceres::abs(asqb) < Eps)) {
      return false;
    }

    T br   = ceres::sqrt(asqr - T(1.));
    T bg   = ceres::sqrt(asqg - T(1.));
    T bb   = ceres::sqrt(asqb - T(1.));
    T bShr = br * Sr * d;
    T bShg = bg * Sg * d;
    T bShb = bb * Sb * d;

    if ((ceres::abs(bShr) < Eps) || (ceres::abs(bShg) < Eps) ||
        (ceres::abs(bShb) < Eps)) {
      return false;
    }

    T bcothbShr = br * ceres::coth(bShr);
    T bcothbShg = bg * ceres::coth(bShg);
    T bcothbShb = bb * ceres::coth(bShb);
    T Rr = (T(1.) - _R0[0] * (ar - bcothbShr)) / (ar - _R0[0] + bcothbShr);
    T Rg = (T(1.) - _R0[1] * (ag - bcothbShg)) / (ag - _R0[1] + bcothbShg);
    T Rb = (T(1.) - _R0[2] * (ab - bcothbShb)) / (ab - _R0[2] + bcothbShb);

    residuals[0] = Rr - _R1[0];
    residuals[1] = Rg - _R1[1];
    residuals[2] = Rb - _R1[2];

    return true;
  }

  // Factory to hide the construction of the CostFunction object from
  // the client code.
  static ::ceres::CostFunction* Create(const ::painty::vec3& R0,
                                       const ::painty::vec3& R1) {
    auto* c =
      (new ::ceres::AutoDiffCostFunction<CostFunction_KS_single_d, 3, 3, 3, 1>(
        new CostFunction_KS_single_d(R0, R1)));

    return c;
  }

  ::painty::vec3 _R0;
  ::painty::vec3 _R1;
};

struct CostFunction_single_d {
  CostFunction_single_d(const ::painty::PaintCoeff& paint,
                        const ::painty::vec3& R0, const ::painty::vec3& R1)
      : _paint(paint),
        _R0(R0),
        _R1(R1) {}

  template <typename T>
  bool operator()(const T* const thickness, T* residuals) const {
    const T Eps = T(1e-9);

    T Kr = T(_paint.K[0]);
    T Kg = T(_paint.K[1]);
    T Kb = T(_paint.K[2]);
    T Sr = T(_paint.S[0]);
    T Sg = T(_paint.S[1]);
    T Sb = T(_paint.S[2]);
    T d  = thickness[0];

    if ((ceres::abs(Kr) <= Eps) || (ceres::abs(Kg) < Eps) ||
        (ceres::abs(Kb) < Eps) || (ceres::abs(Sr) < Eps) ||
        (ceres::abs(Sg) < Eps) || (ceres::abs(Sb) < Eps) ||
        (ceres::abs(d) < Eps)) {
      return false;
    }

    // compute reflectance
    T ar   = T(1.) + Kr / Sr;
    T ag   = T(1.) + Kg / Sg;
    T ab   = T(1.) + Kb / Sb;
    T asqr = ceres::pow(ar, T(2));
    T asqg = ceres::pow(ag, T(2));
    T asqb = ceres::pow(ab, T(2));

    if ((ceres::abs(asqr) < Eps) || (ceres::abs(asqg) < Eps) ||
        (ceres::abs(asqb) < Eps)) {
      return false;
    }

    T br   = ceres::sqrt(asqr - T(1.));
    T bg   = ceres::sqrt(asqg - T(1.));
    T bb   = ceres::sqrt(asqb - T(1.));
    T bShr = br * Sr * d;
    T bShg = bg * Sg * d;
    T bShb = bb * Sb * d;

    if ((ceres::abs(bShr) < Eps) || (ceres::abs(bShg) < Eps) ||
        (ceres::abs(bShb) < Eps)) {
      return false;
    }

    T bcothbShr = br * ceres::coth(bShr);
    T bcothbShg = bg * ceres::coth(bShg);
    T bcothbShb = bb * ceres::coth(bShb);
    T Rr = (T(1.) - _R0[0] * (ar - bcothbShr)) / (ar - _R0[0] + bcothbShr);
    T Rg = (T(1.) - _R0[1] * (ag - bcothbShg)) / (ag - _R0[1] + bcothbShg);
    T Rb = (T(1.) - _R0[2] * (ab - bcothbShb)) / (ab - _R0[2] + bcothbShb);

    residuals[0] = Rr - _R1[0];
    residuals[1] = Rg - _R1[1];
    residuals[2] = Rb - _R1[2];

    return true;
  }

  // Factory to hide the construction of the CostFunction object from
  // the client code.
  static ::ceres::CostFunction* Create(const ::painty::PaintCoeff& paint,
                                       const ::painty::vec3& R0,
                                       const ::painty::vec3& R1) {
    auto* c = (new ::ceres::AutoDiffCostFunction<CostFunction_single_d, 3, 1>(
      new CostFunction_single_d(paint, R0, R1)));

    return c;
  }

  ::painty::PaintCoeff _paint;
  ::painty::vec3 _R0;
  ::painty::vec3 _R1;
};

}  //namespace MeasureSolver
namespace painty {

auto extractThickness(const Mat3d& R0_in, const Mat3d& R1_in,
                      const std::vector<Mat1d>& masks, const Palette& palette)
  -> Mat1d {
  Mat3d R0, R1;
  cv::GaussianBlur(R0_in, R0, cv::Size(-1, -1), 0.5);
  cv::GaussianBlur(R1_in, R1, cv::Size(-1, -1), 0.5);

  // For each individual paint separated by the masks
  // measure the K and S values by solving non linear least squares
  Mat1d thicknessMap = Mat1d(R0.size(), 0.);

  size_t pp = 0UL;
  for (const Mat1d& sampleMask : masks) {
    const auto& paint = palette[pp++];

    std::vector<vec3> samples_R0;
    std::vector<vec3> samples_R1;
    std::vector<size_t> point_indices;

    ceres::Problem problem;
    int32_t nSamples = 0;

    for (int32_t i = 0; i < static_cast<int32_t>(R0.total()); i++) {
      if (!(sampleMask(i) > 0.0) ||
          (R0(i) - R1(i)).norm() <= MeasureSolver::measure_threshold) {
        continue;
      }
      nSamples++;

      point_indices.push_back(static_cast<size_t>(i));
      samples_R0.emplace_back(R0(i)[0], R0(i)[1], R0(i)[2]);
      samples_R1.emplace_back(R1(i)[0], R1(i)[1], R1(i)[2]);

      ::ceres::CostFunction* dataCostFunction =
        MeasureSolver::CostFunction_single_d::Create(
          paint, samples_R0[static_cast<size_t>(i)],
          samples_R1[static_cast<size_t>(i)]);
      double* dm = &(thicknessMap(i));
      *dm        = 1.;
      problem.AddResidualBlock(dataCostFunction, nullptr, dm);

      problem.SetParameterLowerBound(dm, 0, MeasureSolver::d_bound_lower);
      problem.SetParameterUpperBound(dm, 0, MeasureSolver::d_bound_upper);
    }

    if (nSamples <= 0) {
      std::cerr << "couldn't find samples" << std::endl;
      continue;
    }

    ::ceres::Solver::Options options;
    options.minimizer_progress_to_stdout = true;
    options.num_threads =
      static_cast<int32_t>(std::thread::hardware_concurrency());
    options.num_linear_solver_threads =
      static_cast<int32_t>(std::thread::hardware_concurrency());
    options.max_num_iterations = MeasureSolver::max_iterations;
    options.function_tolerance = MeasureSolver::function_tolerance;

    ::ceres::Solver::Summary summary;
    ::ceres::Solve(options, &problem, &summary);
  }

  return thicknessMap;
}

auto createPaletteFromReflectanceData(const Mat3d& R0_in, const Mat3d& R1_in,
                                      const std::vector<Mat1d>& masks,
                                      Mat1d& thicknessMap,
                                      bool estimateSingleThickness) -> Palette {
  Palette palette = {};

  Mat3d R0, R1;
  cv::GaussianBlur(R0_in, R0, cv::Size(-1, -1), 0.5);
  cv::GaussianBlur(R1_in, R1, cv::Size(-1, -1), 0.5);

  // For each individual paint separated by the masks
  // measure the K and S values by solving non linear least squares
  thicknessMap = Mat1d(R0.size(), 0.);

  for (const Mat1d& sampleMask : masks) {
    vec3 K = {0.5, 0.5, 0.5};
    vec3 S = {0.5, 0.5, 0.5};

    std::vector<vec3> samples_R0;
    std::vector<vec3> samples_R1;
    std::vector<size_t> point_indices;

    ceres::Problem problem;
    int32_t nSamples = 0;

    double meanThickness = 1;

    double* d = nullptr;
    for (int32_t i = 0; i < static_cast<int32_t>(R0.total()); i++) {
      if (!(sampleMask(i) > 0.0) ||
          (R0(i) - R1(i)).norm() <= MeasureSolver::measure_threshold) {
        continue;
      }

      nSamples++;

      point_indices.push_back(static_cast<size_t>(i));
      samples_R0.emplace_back(R0(i)[0], R0(i)[1], R0(i)[2]);
      samples_R1.emplace_back(R1(i)[0], R1(i)[1], R1(i)[2]);

      ::ceres::CostFunction* dataCostFunction;
      if (estimateSingleThickness) {
        dataCostFunction = MeasureSolver::CostFunction_KS_single_d::Create(
          samples_R0.back(), samples_R1.back());
        d = &meanThickness;
        problem.AddResidualBlock(dataCostFunction, nullptr, K.data(), S.data(),
                                 d);
      } else {
        dataCostFunction = MeasureSolver::CostFunction_KS_d_map::Create(
          samples_R0.back(), samples_R1.back());
        double* dm = &(thicknessMap(i));
        *dm        = 1.;
        problem.AddResidualBlock(dataCostFunction, nullptr, K.data(), S.data(),
                                 dm);
        problem.SetParameterLowerBound(dm, 0, MeasureSolver::d_bound_lower);
        problem.SetParameterUpperBound(dm, 0, MeasureSolver::d_bound_upper);
      }
    }
    if (estimateSingleThickness) {
      problem.SetParameterLowerBound(d, 0, MeasureSolver::d_bound_lower);
      problem.SetParameterUpperBound(d, 0, MeasureSolver::d_bound_upper);
    }

    if (nSamples <= 0) {
      LOG(ERROR) << "couldn't find samples" << std::endl;
      continue;
    }

    problem.SetParameterLowerBound(K.data(), 0, MeasureSolver::KS_lower_bound);
    problem.SetParameterUpperBound(K.data(), 0, MeasureSolver::KS_upper_bound);
    problem.SetParameterLowerBound(S.data(), 0, MeasureSolver::KS_lower_bound);
    problem.SetParameterUpperBound(S.data(), 0, MeasureSolver::KS_upper_bound);
    problem.SetParameterLowerBound(K.data(), 1, MeasureSolver::KS_lower_bound);
    problem.SetParameterUpperBound(K.data(), 1, MeasureSolver::KS_upper_bound);
    problem.SetParameterLowerBound(S.data(), 1, MeasureSolver::KS_lower_bound);
    problem.SetParameterUpperBound(S.data(), 1, MeasureSolver::KS_upper_bound);
    problem.SetParameterLowerBound(K.data(), 2, MeasureSolver::KS_lower_bound);
    problem.SetParameterUpperBound(K.data(), 2, MeasureSolver::KS_upper_bound);
    problem.SetParameterLowerBound(S.data(), 2, MeasureSolver::KS_lower_bound);
    problem.SetParameterUpperBound(S.data(), 2, MeasureSolver::KS_upper_bound);

    ::ceres::Solver::Options options;
    options.minimizer_progress_to_stdout = true;
    options.num_threads =
      static_cast<int32_t>(std::thread::hardware_concurrency());
    options.num_linear_solver_threads =
      static_cast<int32_t>(std::thread::hardware_concurrency());
    options.max_num_iterations = MeasureSolver::max_iterations;
    options.function_tolerance = MeasureSolver::function_tolerance;

    ::ceres::Solver::Summary summary;
    ::ceres::Solve(options, &problem, &summary);
    // LOG(INFO) << summary.BriefReport() << std::endl;

    palette.push_back({K, S});
  }

  return palette;
}

auto getThinningMedium() -> PaintCoeff {
  painty::Palette palette = {};
  {
    constexpr auto palette_file = "data/paint_palettes/thinning_medium.json";
    auto fin                    = std::ifstream();
    fin.open(palette_file);
    painty::LoadPalette(fin, palette);
    fin.close();
  }
  return palette.front();
}

}  // namespace painty
