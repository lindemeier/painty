/**
 * @file SbrPainterTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-09-11
 *
 */

#include <fstream>

#include "gtest/gtest.h"
#include "painty/io/ImageIO.hxx"
#include "painty/mixer/Serialization.hxx"
#include "painty/renderer/BrushBase.hxx"
#include "painty/renderer/FootprintBrush.hxx"
#include "painty/renderer/Renderer.hxx"
#include "painty/renderer/TextureBrush.hxx"
#include "painty/sbr/PictureTargetSbrPainter.hxx"

TEST(SbrPainterTest, Construct) {
  painty::Palette palette = {};
  {
    constexpr auto palette_file =
      "data/paint_palettes/lindemeier-measured.json";
    auto fin = std::ifstream();
    fin.open(palette_file);
    painty::LoadPalette(fin, palette);
    fin.close();
  }

  painty::Mat3d image;
  painty::io::imRead("./data/test_images/field.jpg", image, false);
  image = painty::ScaledMat(image, image.rows / 4, image.cols / 4);

  auto brushPtr =
    std::make_shared<painty::TextureBrush<painty::vec3>>("data/sample_0");

  // auto brushPtr = std::make_shared<painty::FootprintBrush<painty::vec3>>(120.0);

  auto canvasPtr =
    std::make_shared<painty::Canvas<painty::vec3>>(image.rows, image.cols);
  painty::PictureTargetSbrPainter picturePainter(
    canvasPtr, std::make_shared<painty::PaintMixer>(palette), brushPtr);

  picturePainter._paramsInput.inputSRGB = image;

  picturePainter.paint();

  painty::Renderer<painty::vec3> renderer;
  painty::io::imSave("/tmp/canvasComposed.png", renderer.compose(*canvasPtr),
                     true);
}
