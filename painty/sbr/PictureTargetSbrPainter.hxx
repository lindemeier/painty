/**
 * @file PictureTargetSbrPainter.hxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-09-11
 *
 */
#pragma once

#include <memory>

#include "painty/image/Superpixel.hxx"
#include "painty/mixer/PaintMixer.hxx"
#include "painty/renderer/SbrRenderThread.hxx"

namespace painty {
class PictureTargetSbrPainter {
 public:
  struct ParamsInput {
    Mat3d inputSRGB;  // target image
    Mat1d mask;       // mask for marking areas that should be skipped
    double sigmaSpatial       = 3.0;   // bilateral filter spatial sigma
    double sigmaColor         = 4.25;  // bilateral filter color sigma
    uint32_t smoothIterations = 5U;    // bilateral filter color sigma
    uint32_t nrColors         = 6U;
    double thinningVolume     = 2.0;
    double alphaDiff =
      0.75;  // weight color diff in contrast to derivative diff
  };

  struct ParamsOrientations {
    double innerBlurScale =
      0.0;  // Gaussian blur scale for orientation derivatives
    double outerBlurScale = 1.0;  // Gaussian blur scale for orientation tensors
  };

  struct ParamsStroke {
    std::vector<double> brushSizes = {60.0, 30.0, 10.0};
    uint32_t minLen                = 5U;  // min number of stroke control points
    uint32_t maxLen = 12U;                // max number of stroke control points
    double stepSize =
      0.0;  // step size used for tracing stroke paths ( <= 0) for step size computed from brush radius
    double curvatureAlpha =
      1.0;  // stroke smoothness (0 for straight, 1 for smooth curved strokes)
    bool blockVisitedRegions =
      true;  // block regions from stroke seeding when the have been visited by another, previously generated stroke
    bool clampBrushRadius = true;  //
    double thicknessScale = 2.0;
  };

  struct ParamsConvergence {
    uint32_t maxIterations = 2U;  // maximum number of layers
    double rms_local =
      0.1;  // maximum root mean square error for evaluation regions
    double rms_global = 0.0;  // maximum root mean square error the whole image
  };

  struct ParamsPaintSequence {
    double dipAfter =
      400.0;  // maximum length of painting movement before the brush will be dipped in paint again
  };

  struct ParamsRegionExtraction {
    bool useDiffWeights = true;
    SuperpixelSegmentation::ExtractionStrategy extractionStrategy =
      SuperpixelSegmentation::ExtractionStrategy::SLICO_POISSON_WEIGHTED;
  };

  PictureTargetSbrPainter(
    const std::shared_ptr<GpuTaskQueue>& gpuTaskQueue, const Size& rendererSize,
    const std::shared_ptr<PaintMixer>& basePigmentsMixerPtr);

  auto paint() -> Mat3d;

  PictureTargetSbrPainter() = delete;

  void enableCoatCanvas(bool enable);
  void enableSmudge(bool enable);

  ParamsInput _paramsInput;
  ParamsOrientations _paramsOrientations;
  ParamsStroke _paramsStroke;
  ParamsConvergence _paramsConvergence;
  ParamsPaintSequence _paramsPaintSequence;
  ParamsRegionExtraction _paramsRegionExtraction;

 private:
  static constexpr auto BrushMinSize        = 2.0;
  static constexpr auto BrushMaxSize        = 1000.0;
  static constexpr auto AssumedAvgThickness = 0.5;

  struct BrushStroke {
    std::vector<vec2> path = {};
    double radius          = 0.0;
    PaintCoeff paint;
  };

  using ColorIndexBrushStrokeMap = std::map<size_t, std::vector<BrushStroke>>;

  auto extractRegions(const Mat3d& target_Lab, const Mat1d& difference,
                      double brushSize) const
    -> std::pair<Mat<int32_t>, std::map<int32_t, ImageRegion>>;

  auto checkConvergence(const Mat1d& difference,
                        std::map<int32_t, ImageRegion>& regions,
                        Mat<int32_t>& labels, const double epsFac) const
    -> bool;

  auto generateBrushStrokes(std::map<int32_t, ImageRegion>& regions,
                            const Mat3d& target_Lab,
                            const Mat3d& canvasCurrentLab,
                            const Mat1d& difference, double brushRadius,
                            const Palette& palette, const Mat<int32_t>& labels,
                            const Mat1d& mask, const Mat3d& tensors) const
    -> ColorIndexBrushStrokeMap;

  auto findBestPaintIndex(const vec3& R_target, const vec3& R0,
                          const Palette& palette) const
    -> std::optional<size_t>;

  void paintCoatCanvas(const PaintCoeff& paint);

  auto computeDifference(const Mat3d& target_Lab, const Mat3d& canvasCurrentLab,
                         const double brushRadius) const -> Mat1d;

  SbrRenderThread _renderThread;

  std::shared_ptr<PaintMixer> _basePigmentsMixerPtr = nullptr;
  bool _coatCanvas = false;
};
}  // namespace painty
