/**
 * @file TextureBrushDictionary.cxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-09-29
 *
 */
#include "painty/renderer/TextureBrushDictionary.hxx"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <random>

// #include "opencv2/highgui/highgui.hpp"
#include "painty/io/ImageIO.hxx"

namespace painty {

TextureBrushDictionary::TextureBrushDictionary() {
  createBrushTexturesFromFolder("data/textures");
}

auto TextureBrushDictionary::lookup(const std::vector<vec2>& path,
                                    const double brushSize) const -> Entry {
  auto length = 0.0;
  for (auto i = 0U; i < (path.size() - 1U); i++) {
    length += (path[i] - path[i + 1U]).norm();
  }

  uint32_t i0 = 0;
  uint32_t i1 = 1;

  // find best fitting size
  double mr = _avgSizes[0U];
  for (uint32_t i = 0U; i < _avgSizes.size(); i++) {
    double d = std::abs(_avgSizes[i] - brushSize);
    if (d < mr) {
      mr = d;
      i0 = i;
    }
  }

  // find best fitting length
  double ml = _avgTexLength[i0][0];
  for (uint32_t i = 0U; i < _avgSizes.size(); i++) {
    double d = abs(_avgTexLength[i0][i] - length);
    if (d < ml) {
      ml = d;
      i1 = i;
    }
  }

  const auto& candidates = _brushTexturesBySizeByLength[i0][i1];

  if (candidates.empty()) {
    throw std::runtime_error("no candidate found");
  }

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<std::size_t> dis(
    static_cast<std::size_t>(0UL),
    candidates.size() - static_cast<std::size_t>(1UL));
  const auto index = dis(gen);

  return candidates[index];
}

auto TextureBrushDictionary::loadHeightMap(const std::string& file) const
  -> Mat1d {
  Mat1d gray;
  io::imRead(file, gray, false);

  cv::normalize(gray, gray, 0.0, 1.0, cv::NORM_MINMAX);

  return gray;
}

void TextureBrushDictionary::createBrushTexturesFromFolder(
  const std::string& folder) {
  auto split = [](const std::string& input, const char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(input);
    std::string item;
    while (std::getline(ss, item, delim)) {
      elems.push_back(item);
    }
    return elems;
  };

  std::map<uint32_t, std::map<uint32_t, std::vector<Entry>>> textureMap;

  for (const auto& p : std::filesystem::directory_iterator(folder)) {
    const auto filepath = p.path();
    // std::cout << filepath << std::endl;

    const std::string filename =
      split(split(filepath, '.').front(), '/').back();

    std::vector<std::string> tokens = split(filename, '_');
    const auto radius = static_cast<uint32_t>(std::stoi(tokens[0]));
    const auto length = static_cast<uint32_t>(std::stoi(tokens[1]));

    Entry entry;
    entry.texHost = loadHeightMap(filepath);

    Mat1f fImage = {};
    {
      Mat1f copy = {};
      cv::normalize(entry.texHost, copy, 0.0, 1.0, cv::NORM_MINMAX);
      cv::flip(copy, copy, 0);
      copy.convertTo(fImage, CV_32FC1, 1.0);
    }

    // cv::imshow("brush_tex", byteImage);
    // cv::waitKey(100);

    entry.texGpu = prgl::Texture2d::Create(
      fImage.cols, fImage.rows, prgl::TextureFormatInternal::R32F,
      prgl::TextureFormat::Red, prgl::DataType::Float);
    entry.texGpu->upload(fImage.data);

    textureMap[radius][length].push_back(entry);
  }

  _brushTexturesBySizeByLength.clear();
  for (const auto& e : textureMap) {
    _brushTexturesBySizeByLength.push_back(std::vector<std::vector<Entry>>());
    for (const auto& a : e.second) {
      _brushTexturesBySizeByLength.back().push_back(std::vector<Entry>());
      for (const auto& tex : a.second) {
        _brushTexturesBySizeByLength.back().back().push_back(tex);
      }
    }
  }

  _avgSizes.resize(_brushTexturesBySizeByLength.size());
  for (auto& e : _avgSizes) {
    e = 0.0;
  }
  std::vector<uint32_t> brushSizesCounters(_brushTexturesBySizeByLength.size(),
                                           0U);
  for (auto i = 0U; i < _brushTexturesBySizeByLength.size(); i++) {
    for (auto j = 0U; j < _brushTexturesBySizeByLength[i].size(); j++) {
      for (const auto& texture : _brushTexturesBySizeByLength[i][j]) {
        _avgSizes[i] += static_cast<double>(texture.texHost.rows);
        brushSizesCounters[i]++;
      }
    }
  }
  for (auto i = 0U; i < _avgSizes.size(); i++) {
    _avgSizes[i] *= (1.0 / static_cast<double>(brushSizesCounters[i]));
  }

  _avgTexLength.resize(_brushTexturesBySizeByLength.size());
  std::vector<std::vector<uint32_t>> brushLengthCounters(
    _brushTexturesBySizeByLength.size());
  for (auto i = 0U; i < _brushTexturesBySizeByLength.size(); i++) {
    _avgTexLength[i] =
      std::vector<double>(_brushTexturesBySizeByLength[i].size(), 0.0);
    brushLengthCounters[i] =
      std::vector<uint32_t>(_brushTexturesBySizeByLength[i].size(), 0U);
  }
  for (auto i = 0U; i < _brushTexturesBySizeByLength.size(); i++) {
    for (auto j = 0U; j < _brushTexturesBySizeByLength[i].size(); j++) {
      for (const auto& texture : _brushTexturesBySizeByLength[i][j]) {
        _avgTexLength[i][j] += static_cast<double>(texture.texHost.cols);
        brushLengthCounters[i][j]++;
      }
    }
  }
  for (auto i = 0U; i < _avgTexLength.size(); i++) {
    for (auto j = 0U; j < _avgTexLength[i].size(); j++) {
      _avgTexLength[i][j] *=
        (1.0 / static_cast<double>(brushLengthCounters[i][j]));
    }
  }
}

}  // namespace painty
