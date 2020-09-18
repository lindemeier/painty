/**
 * @file PictureTargetSbrPainter.cxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-09-11
 *
 */
#include "painty/sbr/PictureTargetSbrPainter.hxx"

#include <random>

#include "painty/core/Color.hxx"
#include "painty/image/Convolution.hxx"
#include "painty/image/EdgeTangentFlow.hxx"
#include "painty/io/ImageIO.hxx"
#include "painty/mixer/Serialization.hxx"
#include "painty/renderer/Renderer.hxx"
#include "painty/sbr/PathTracer.hxx"

namespace painty {
PictureTargetSbrPainter::PictureTargetSbrPainter(
  const std::shared_ptr<Canvas<vec3>>& canvasPtr,
  const std::shared_ptr<PaintMixer>& basePigmentsMixerPtr,
  const std::shared_ptr<BrushBase<vec3>>& painterPtr)
    : _canvasPtr(canvasPtr),
      _basePigmentsMixerPtr(basePigmentsMixerPtr),
      _brushPtr(painterPtr) {}

auto PictureTargetSbrPainter::extractRegions(const Mat3d& target_Lab,
                                             const Mat3d& canvasCurrentLab,
                                             const Mat1d& difference,
                                             double brushSize) const
  -> std::pair<Mat<int32_t>, std::map<int32_t, ImageRegion>> {
  Mat3d segImage = _paramsInput.inputSRGB.clone();
  Mat3d segDiffImage(segImage.size());
  std::map<int32_t, ImageRegion> regions;
  Mat<int32_t> labels;

  SuperpixelSegmentation seg;
  seg.setUseDiffWeight(_paramsRegionExtraction.useDiffWeights);
  seg.setExtractionStrategy(_paramsRegionExtraction.extractionStrategy);
  seg.extract(target_Lab, canvasCurrentLab, _paramsInput.mask,
              static_cast<int32_t>(brushSize));
  labels = seg.getRegions(regions);
  for (auto j = 0; j < static_cast<int32_t>(segDiffImage.total()); ++j) {
    segDiffImage(j)[0] = difference(j);
    segDiffImage(j)[1] = difference(j);
    segDiffImage(j)[2] = difference(j);
  }
  seg.getSegmentationOutlined(segImage);
  seg.getSegmentationOutlined(segDiffImage);
  painty::io::imSave("/tmp/superpixelsTarget.jpg", segImage, false);
  painty::io::imSave("/tmp/superpixelsDifference.jpg", segDiffImage, false);

  return {labels, regions};
}

auto PictureTargetSbrPainter::checkConvergence(
  const Mat1d& difference, std::map<int32_t, ImageRegion>& regions,
  Mat<int32_t>& labels) const -> bool {
  auto globalRMS = 0.0;
  std::cout << "filtering evaluation regions for finished regions" << std::endl;
  auto nrActiveRegions = 0UL;
  for (auto iter = regions.begin(); iter != regions.end(); iter++) {
    auto rms = iter->second.computeRms(difference);

    if (rms >= _paramsConvergence.rms_local) {
      iter->second.setActive(true);
      globalRMS += rms;
      nrActiveRegions++;
    } else {
      iter->second.setActive(false);
      iter->second.fill(labels, -1);
      std::cout << "Region " << iter->first << " set inactive with rms " << rms
                << " < " << _paramsConvergence.rms_local << std::endl;
    }
  }
  globalRMS /= static_cast<double>(nrActiveRegions);
  if (globalRMS < _paramsConvergence.rms_global) {
    std::cout << "Converged globally..." << std::endl;
    return true;
  }
  return false;
}

auto PictureTargetSbrPainter::generateBrushStrokes(
  std::map<int32_t, ImageRegion>& regions, const Mat3d& target_Lab,
  const Mat3d& canvasCurrentLab, const Mat1d& /*difference*/,
  const double brushRadius, const Palette& palette, const Mat<int32_t>& labels,
  const Mat1d& mask) const
  -> PictureTargetSbrPainter::ColorIndexBrushStrokeMap {
  std::cout << "Computing structure tensor field" << std::endl;
  // compute structure tensor field
  const auto tensors =
    tensor::ComputeTensors(target_Lab, _paramsInput.mask,
                           brushRadius * _paramsOrientations.innerBlurScale,
                           brushRadius * _paramsOrientations.outerBlurScale);
  painty::io::imSave("/tmp/targetImageOrientation.jpg",
                     lineIntegralConv(ComputeEdgeTangentFlow(tensors), 10.),
                     false);

  PathTracer tracer(tensors);
  tracer.setMinLen(_paramsStroke.minLen);
  tracer.setMaxLen(_paramsStroke.maxLen);
  tracer.setStep((_paramsStroke.stepSize <= 0.0) ? (brushRadius * 0.5)
                                                 : _paramsStroke.stepSize);
  tracer.setFrame(cv::Rect2i(0, 0, target_Lab.cols, target_Lab.rows));
  tracer.setFc(_paramsStroke.curvatureAlpha);

  ColorIndexBrushStrokeMap brushStrokes;

  std::cout << "Iterating through all active regions" << std::endl;
  for (auto reg : regions) {
    auto& region = reg.second;

    if (!region.isActive()) {
      continue;
    }

    vec2 incenter    = {100, 100};
    auto usedRadius  = region.getInscribedCircle(incenter);
    const auto width = usedRadius * 2.0;

    // TODO clamp is bad for strokes whose minsize is larger than inscribed circle radius.
    if (_paramsStroke.clampBrushRadius) {
      usedRadius = clamp(width, BrushMinSize, BrushMaxSize) / 2.;
    } else {
      if (width < BrushMinSize) {
        region.setActive(false);
        continue;
      }
      if (width > BrushMaxSize) {
        usedRadius = std::min(width, BrushMaxSize) / 2.0;
      }
    }

    vec3 Rt = region.computeMean(target_Lab);
    vec3 R0 = region.computeMean(canvasCurrentLab);
    ColorConverter<double> con;
    con.lab2rgb(R0, R0);
    con.lab2rgb(Rt, Rt);
    auto currentPaintIndex = findBestPaintIndex(Rt, R0, palette);
    if (!currentPaintIndex) {
      continue;
    }

    tracer.setEvaluatePositionFun([&](const vec2& p) -> PathTracer::NextAction {
      if ((static_cast<int32_t>(p[0U]) < 0) ||
          (static_cast<int32_t>(p[1U]) < 0) ||
          (static_cast<int32_t>(p[0U]) >= labels.cols) ||
          (static_cast<int32_t>(p[1U]) >= labels.rows)) {
        return PathTracer::NextAction::PATH_STOP_NOW;
      }
      const auto clabel =
        labels(static_cast<int32_t>(p[1U]), static_cast<int32_t>(p[0U]));

      if ((clabel < 0) ||
          ((!mask.empty()) && (mask(static_cast<int32_t>(p[1U]),
                                    static_cast<int32_t>(p[0U])) < 1.0)) ||
          (regions.count(clabel) == 0u)) {
        return PathTracer::NextAction::PATH_STOP_NOW;
      }

      vec3 LabCanvas = regions[clabel].computeMean(canvasCurrentLab);
      vec3 LabSource = regions[clabel].computeMean(target_Lab);
      ColorConverter<double> converter;
      vec3 R0_test;
      converter.lab2rgb(LabCanvas, R0_test);
      const auto R1 = ComputeReflectance(palette[currentPaintIndex.value()].K,
                                         palette[currentPaintIndex.value()].S,
                                         R0_test, AssumedAvgThickness);

      vec3 Lab1;
      converter.rgb2lab(R1, Lab1);

      if ((Lab1 - LabSource).squaredNorm() <
          (LabSource - LabCanvas).squaredNorm()) {
        return PathTracer::NextAction::PATH_CONTINUE;
      }
      return PathTracer::NextAction::PATH_STOP_NEXT;
    });

    // std::cout << "Generating path at: " << incenter.transpose() << std::endl;
    auto path = tracer.trace(incenter);

    // std::cout << "Adding valid strokes and sort by paint" << std::endl;
    if (_paramsStroke.blockVisitedRegions) {
      for (const auto& p : path) {
        if ((static_cast<int32_t>(p[0U]) < 0) ||
            (static_cast<int32_t>(p[1U]) < 0) ||
            (static_cast<int32_t>(p[0U]) >= labels.cols) ||
            (static_cast<int32_t>(p[1U]) >= labels.rows)) {
          continue;
        }
        // block this cell from painting for future visits
        regions[labels(static_cast<int32_t>(p[1]), static_cast<int32_t>(p[0]))]
          .setActive(false);
      }
    }

    if (!path.empty()) {
      brushStrokes[currentPaintIndex.value()].push_back({path, usedRadius});
    }
  }
  return brushStrokes;
}

auto PictureTargetSbrPainter::findBestPaintIndex(const vec3& R_target,
                                                 const vec3& R0,
                                                 const Palette& palette)
  -> std::optional<size_t> {
  ColorConverter<double> con;
  vec3 R_target_Lab;
  vec3 R0_Lab;
  con.rgb2lab(R_target, R_target_Lab);
  con.rgb2lab(R0, R0_Lab);

  auto d = (R0_Lab - R_target_Lab).squaredNorm();

  size_t bestIndex = std::numeric_limits<size_t>::max();

  for (size_t i = 0UL; (i < palette.size()); i++) {
    vec3 R_Lab;
    con.rgb2lab(
      ComputeReflectance(palette[i].K, palette[i].S, R0, AssumedAvgThickness),
      R_Lab);

    const auto ld = (R_Lab - R_target_Lab).squaredNorm();
    if (ld < d) {
      d         = ld;
      bestIndex = i;
    }
  }

  return (bestIndex == std::numeric_limits<size_t>::max())
           ? std::nullopt
           : std::optional<size_t>(bestIndex);
}

auto PictureTargetSbrPainter::computeDifference(const Mat3d& target_Lab,
                                                const Mat3d& canvasCurrentLab)
  -> Mat1d {
  auto difference = Mat1d(target_Lab.size());
  for (auto i = 0; i < static_cast<int32_t>(target_Lab.total()); i++) {
    difference(i) = ColorConverter<double>::ColorDifference(
      target_Lab(i), canvasCurrentLab(i));
  }
  painty::io::imSave("/tmp/difference.jpg", difference, false);
  return difference;
}

auto PictureTargetSbrPainter::paint() -> bool {
  if (_paramsInput.inputSRGB.empty()) {
    throw std::runtime_error("_paramsInput.inputSRGB.empty()");
  }
  std::cout << "Converting input to Lab and apply smoothing" << std::endl;
  // convert to CIELab and blur the image using a bilateral filter
  const auto target_Lab = smoothOABF(
    convertColor(_paramsInput.inputSRGB,
                 ColorConverter<double>::Conversion::srgb_2_CIELab),
    Mat1d(), _paramsInput.sigmaSpatial, _paramsInput.sigmaColor,
    _paramsOrientations.outerBlurScale, _paramsInput.smoothIterations);

  painty::io::imSave(
    "/tmp/targetImage.jpg",
    convertColor(target_Lab, ColorConverter<double>::Conversion::CIELab_2_srgb),
    false);

  std::cout << "Extract color palette from image using base pigments"
            << std::endl;
  // mix palette for the image from the painters base pigments
  auto palette = _basePigmentsMixerPtr->mixFromInputPicture(
    _paramsInput.inputSRGB, _paramsInput.nrColors);
  // make the paints thinner
  {
    const auto thinner = getThinningMedium();
    for (auto& paint : palette) {
      paint = _basePigmentsMixerPtr->mixed(paint, 1.0, thinner,
                                           _paramsInput.thinningVolume);
    }
  }
  painty::io::imSave("/tmp/targetImagePalette.jpg",
                     VisualizePalette(palette, 1.0), false);

  // for every brush
  for (const auto brushSize : _paramsStroke.brushSizes) {
    std::cout << "Switching to brush size: " << brushSize << std::endl;
    const double brushRadius = brushSize / 2.0;

    std::cout << "Iterating layers" << std::endl;

    for (uint32_t iteration = 0U; iteration < _paramsConvergence.maxIterations;
         iteration++) {
      std::cout << "Iteration: " << iteration << std::endl;

      std::cout << "Getting current state of the canvas" << std::endl;
      const auto canvasCurrentRGBLinear = Renderer<vec3>().compose(*_canvasPtr);
      painty::io::imSave("/tmp/canvasCurrent.jpg", canvasCurrentRGBLinear,
                         true);
      const auto canvasCurrentLab = ScaledMat(
        convertColor(canvasCurrentRGBLinear,
                     ColorConverter<double>::Conversion::rgb_2_CIELab),
        target_Lab.rows, target_Lab.cols);

      std::cout << "Compute difference of target and canvas" << std::endl;
      auto difference = computeDifference(target_Lab, canvasCurrentLab);

      std::cout << "Extracting superpixels" << std::endl;
      std::map<int32_t, ImageRegion> regions;
      Mat<int32_t> labels;
      std::tie(labels, regions) =
        extractRegions(target_Lab, canvasCurrentLab, difference, brushSize);

      // discard already close enough regions
      if (checkConvergence(difference, regions, labels)) {
        return true;
      }

      auto brushStrokeMap =
        generateBrushStrokes(regions, target_Lab, canvasCurrentLab, difference,
                             brushRadius, palette, labels, _paramsInput.mask);
      std::cout << "Rendering strokes" << std::endl;
      const auto xs = static_cast<double>(_canvasPtr->getR0().cols) /
                      static_cast<double>(target_Lab.cols);
      const auto ys = static_cast<double>(_canvasPtr->getR0().rows) /
                      static_cast<double>(target_Lab.rows);
      for (auto& element : brushStrokeMap) {
        const auto paint = palette[element.first];
        std::cout << "Changing paint to: " << element.first << std::endl;
        _brushPtr->dip({paint.K, paint.S});
        for (auto& brushStroke : element.second) {
          for (auto& vertex : brushStroke.path) {
            vertex[0U] *= xs;
            vertex[1U] *= ys;
          }
          _brushPtr->setRadius(((xs + ys) * 0.5) * brushStroke.radius);
          // std::cout << "brush path length: " << brushStroke.path.size()
          //           << std::endl;
          _brushPtr->paintStroke(brushStroke.path, *_canvasPtr);
        }
      }
    }
  }

  return true;
}

}  // namespace painty
