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
#include "painty/mixer/Serialization.hxx"
#include "painty/renderer/Renderer.hxx"
#include "painty/sbr/PictureTargetSbrPainter.hxx"
#include "painty/sbr/SbrPainter.hxx"

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

  auto canvasPtr = std::make_shared<painty::Canvas<painty::vec3>>(768, 1024);

  auto painterPtr = std::make_shared<painty::SbrPainterTextureBrush>();

  painterPtr->setBrushRadius(13.0);
  painterPtr->dipBrush({palette.front().K, palette.front().S});
  painterPtr->paintStroke({{100.0, 100.0}, {200.0, 200.0}, {300.0, 300.0}},
                          *canvasPtr);

  painty::Renderer<painty::vec3> renderer;
  painty::io::imSave("/tmp/canvasComposed.png", renderer.compose(*canvasPtr),
                     true);

  painty::PictureTargetSbrPainter picturePainter(
    canvasPtr, std::make_shared<painty::PaintMixer>(palette), painterPtr);

  painty::Mat3d image;
  painty::io::imRead("./data/test_images/field.jpg", image, false);

  picturePainter.paintImage(image);
}
