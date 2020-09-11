/**
 * @file PictureTargetSbrPainter.cxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-09-11
 *
 */
#include "painty/sbr/PictureTargetSbrPainter.hxx"

#include "painty/image/Convolution.hxx"
#include "painty/io/ImageIO.hxx"
#include "painty/mixer/Serialization.hxx"

namespace painty {
PictureTargetSbrPainter::PictureTargetSbrPainter(
  const std::shared_ptr<Canvas<vec3>>& canvasPtr,
  const std::shared_ptr<PaintMixer>& basePigmentsMixerPtr,
  const std::shared_ptr<SbrPainterBase>& painterPtr)
    : _canvasPtr(canvasPtr),
      _basePigmentsMixerPtr(basePigmentsMixerPtr),
      _painterPtr(painterPtr) {}

auto PictureTargetSbrPainter::paintImage(const Mat3d& srgbImage) -> bool {
  // convert to CIELab and blur the image using a bilateral filter
  const auto targetImage = smoothOABF(
    convertColor(srgbImage, ColorConverter<double>::Conversion::srgb_2_CIELab),
    Mat1d(), 3.0, 4.25, 3.0, 5U);

  painty::io::imSave(
    "/tmp/targetImage.jpg",
    convertColor(targetImage,
                 ColorConverter<double>::Conversion::CIELab_2_srgb),
    false);

  // mix palette for the image from the painters base pigments
  const auto palette =
    _basePigmentsMixerPtr->mixFromInputPicture(srgbImage, 6U);
  painty::io::imSave("/tmp/targetImagePalette.jpg",
                     VisualizePalette(palette, 1.0), false);

  return true;
}

}  // namespace painty
