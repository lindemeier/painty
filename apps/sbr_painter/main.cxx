#include <fstream>
#include <future>
#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "cxxopts.hpp"
#include "nlohmann/json.hpp"
#pragma clang diagnostic pop
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "painty/io/ImageIO.hxx"
#include "painty/mixer/Palette.hxx"
#include "painty/mixer/Serialization.hxx"
#include "painty/renderer/FootprintBrush.hxx"
#include "painty/renderer/Renderer.hxx"
#include "painty/renderer/TextureBrushGpu.hxx"
#include "painty/sbr/PictureTargetSbrPainter.hxx"
#include "prgl/Window.hxx"

namespace painty {
static void from_json(const nlohmann::json& j,
                      PictureTargetSbrPainter::ParamsInput& p) {
  p.sigmaSpatial     = j.value("sigmaSpatial", 3.0);
  p.sigmaColor       = j.value("sigmaColor", 4.25);
  p.smoothIterations = j.value("smoothIterations", 5U);
  p.nrColors         = j.value("nrColors", 6U);
  p.thinningVolume   = j.value("thinningVolume", 1.0);
  p.alphaDiff        = j.value("alphaDiff", 1.0);
}

static void from_json(const nlohmann::json& j,
                      PictureTargetSbrPainter::ParamsOrientations& p) {
  p.innerBlurScale = j.value("innerBlurScale", 0.0);
  p.outerBlurScale = j.value("outerBlurScale", 1.0);
}

static void from_json(const nlohmann::json& j,
                      PictureTargetSbrPainter::ParamsStroke& p) {
  p.brushSizes = j.value("brushSizes", std::vector<double>{60.0, 30.0, 10.0});
  p.minLen     = j.value("minLen", 5U);
  p.maxLen     = j.value("maxLen", 12U);
  p.stepSize   = j.value("stepSize", 0.0);
  p.curvatureAlpha      = j.value("curvatureAlpha", 1.0);
  p.blockVisitedRegions = j.value("blockVisitedRegions", true);
  p.clampBrushRadius    = j.value("clampBrushRadius", true);
  p.thicknessScale      = j.value("thicknessScale", 2.0);
}

static void from_json(const nlohmann::json& j,
                      PictureTargetSbrPainter::ParamsConvergence& p) {
  p.maxIterations = j.value("maxIterations", 3U);
  p.rms_local     = j.value("rms_local", 0.1);
  p.rms_global    = j.value("rms_global", 0.0);
}

}  // namespace painty

int main(int argc, const char* argv[]) {
  cxxopts::Options options(
    argv[0], " - Stroke base renderring painter command line options");
  options.positional_help("[optional args]").show_positional_help();

  options.add_options()
    // clang-format off
      ("i,image", "input picture", cxxopts::value<std::string>())
      ("m,mask", "mask", cxxopts::value<std::string>())
      ("a,canvas", "canvas", cxxopts::value<std::string>())
      ("c,config", "config as json file", cxxopts::value<std::string>())
      ("o,output", "Output file to store the rendered image", cxxopts::value<std::string>()
          ->default_value("sbr.png"))
      ("help", "Print help")
      ;
  // clang-format on

  const auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help({"", "Group"}) << std::endl;
    exit(EXIT_SUCCESS);
  }

  if (result.count("i") == 0UL) {
    std::cerr << "no input picture given" << std::endl;
    std::cout << options.help({"", "Group"}) << std::endl;
    exit(EXIT_FAILURE);
  }

  if (result.count("c") == 0UL) {
    std::cerr << "no config given" << std::endl;
    std::cout << options.help({"", "Group"}) << std::endl;
    exit(EXIT_FAILURE);
  }

  const auto configPath = result["config"].as<std::string>();
  std::cout << "loading config=" << configPath << std::endl;
  nlohmann::json j;
  try {
    std::ifstream jsonFile(configPath);
    if (jsonFile.is_open()) {
      jsonFile >> j;
      jsonFile.close();
    }
  } catch (const std::exception& e) {
    std::cerr << "could not parse " << configPath << " - " << e.what()
              << std::endl;
    exit(EXIT_FAILURE);
  }

  std::cout << "loading base pigments" << std::endl;
  painty::Palette palette = {};
  {
    const auto base_pigments_str =
      j.value("base_pigments", "lindemeier-measured.json");
    const auto palette_file = "data/paint_palettes/" + base_pigments_str;
    auto fin                = std::ifstream();
    fin.open(palette_file);
    painty::LoadPalette(fin, palette);
    fin.close();
  }

  std::cout << "loading target image" << std::endl;
  painty::Mat3d image;
  painty::io::imRead(result["image"].as<std::string>(), image, false);

  constexpr auto OptimRenderSize = 2048U;
  auto width                     = OptimRenderSize;
  auto height                    = OptimRenderSize / 2U;
  if (image.cols > image.rows) {
    width  = OptimRenderSize;
    height = static_cast<uint32_t>(
      (static_cast<double>(width) / static_cast<double>(image.cols)) *
      image.rows);
  } else {
    height = OptimRenderSize;
    width  = static_cast<uint32_t>(
      (static_cast<double>(height) / static_cast<double>(image.rows)) *
      image.cols);
  }
  std::cout << "Creating renderer with width=" << width
            << " and height=" << height << std::endl;




  // const uint32_t dringTimeMillis = j.value("dryingTimeMillis", 1000U * 60U);
  // canvasPtr->setDryingTime(std::chrono::milliseconds(dringTimeMillis));
  // if (result.count("a") == 1UL) {
  //   painty::Mat3d initCanvas;
  //   painty::io::imRead(result["canvas"].as<std::string>(), initCanvas, false);
  //   canvasPtr->setBackground(painty::ScaledMat(initCanvas, height, width));
  // }

  painty::PictureTargetSbrPainter picturePainter(
    painty::Size{width, height}, std::make_shared<painty::PaintMixer>(palette));
  picturePainter.enableCoatCanvas(j.value("coatCanvas", false));
  picturePainter.enableSmudge(j.value("enableSmudge", true));

  std::cout << "Setting configs in renderer" << std::endl;
  picturePainter._paramsInput        = j["image_params"];
  picturePainter._paramsOrientations = j["orientation_params"];
  picturePainter._paramsStroke       = j["stroke_params"];
  picturePainter._paramsConvergence  = j["convergence_params"];

  std::cout << "Setting target image and mask" << std::endl;
  picturePainter._paramsInput.inputSRGB = image;
  if (result.count("m")) {
    painty::Mat1d mask;
    painty::io::imRead(result["mask"].as<std::string>(), mask, false);
    picturePainter._paramsInput.mask = mask;
  } else {
    picturePainter._paramsInput.mask = painty::Mat1d();
  }
  std::cout << "Start painting" << std::endl;

  const auto paintedResult = picturePainter.paint();

  const auto writeFile = result["output"].as<std::string>();
  std::cout << "Writing result to: " << writeFile << std::endl;

  painty::io::imSave(writeFile, paintedResult, true);

  exit(EXIT_SUCCESS);
}
