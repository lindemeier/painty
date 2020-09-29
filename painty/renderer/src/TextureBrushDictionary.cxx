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
#include <random>

#include "painty/io/ImageIO.hxx"

namespace painty {

int32_t TextureBrushDictionaryEntry::getTexSize() const {
  return texture.rows;
}

int32_t TextureBrushDictionaryEntry::getTexLength() const {
  return texture.cols;
}

TextureBrushDictionary::TextureBrushDictionary() {
  createBrushTexturesFromFolder("data/textures");
}

auto TextureBrushDictionary::lookup(const std::vector<vec2>& path,
                                    const double brushSize) const -> Mat1d {
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

  const auto& candidates = _entries[i0][i1];

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<std::size_t> dis(
    static_cast<std::size_t>(0UL),
    candidates.size() - static_cast<std::size_t>(1UL));

  return candidates[dis(gen)].texture;
}

auto TextureBrushDictionary::loadHeightMap(const std::string& file) const
  -> Mat1d {
  std::ifstream heightMapStream(file, std::ios::binary);
  heightMapStream.seekg(0, std::ios::end);
  const auto fileSize = static_cast<std::size_t>(heightMapStream.tellg());
  heightMapStream.seekg(0, std::ios::beg);
  std::vector<uint8_t> data(fileSize);
  heightMapStream.read(reinterpret_cast<char*>(data.data()),
                       static_cast<std::streamsize>(fileSize));

  auto hm = loadHeightMap(data);

  cv::normalize(hm, hm, 0.0, 1.0, cv::NORM_MINMAX);

  return hm;
}

auto TextureBrushDictionary::loadHeightMap(
  const std::vector<uint8_t>& data) const -> Mat1d {
  Mat1d gray;
  io::imRead(data, gray, false);
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

  _entries.clear();

  int32_t radiusC = -1;
  int32_t lengthC = -1;
  int32_t i0      = -1;
  int32_t i1      = -1;

  for (auto& p : std::filesystem::directory_iterator(folder)) {
    const auto filepath = p.path();
    std::cout << filepath << std::endl;

    const std::string filename =
      split(split(filepath, '.').front(), '/').back();

    std::vector<std::string> tokens = split(filename, '_');
    const std::string radius        = tokens[0];
    const std::string length        = tokens[1];
    const std::string id            = tokens[2];

    TextureBrushDictionaryEntry entry;
    entry.radiusNr = std::stoi(radius);
    entry.lengthNr = std::stoi(length);
    entry.id       = std::stoi(id);
    entry.texture  = loadHeightMap(filepath);

    if (entry.radiusNr != radiusC) {
      i0++;
      radiusC = entry.radiusNr;
      lengthC = -1;
      i1      = -1;
    }

    if (entry.lengthNr != lengthC) {
      i1++;
      lengthC = entry.lengthNr;
    }

    while (i0 >= static_cast<int32_t>(_entries.size())) {
      _entries.push_back(
        std::vector<std::vector<TextureBrushDictionaryEntry> >());
    }

    while (i1 >= static_cast<int32_t>(
                   _entries[static_cast<std::size_t>(i0)].size())) {
      _entries[static_cast<std::size_t>(i0)].push_back(
        std::vector<TextureBrushDictionaryEntry>());
    }

    _entries[static_cast<std::size_t>(i0)][static_cast<std::size_t>(i1)]
      .push_back(entry);
  }
  // update sizes and ratios
  _avgSizes.resize(_entries.size());
  _avgTexLength.resize(_entries.size());
  for (auto i = 0U; i < _entries.size(); i++) {
    _avgTexLength[i].resize(_entries[i].size());
    _avgSizes[i] = 0.0;
    int32_t ar   = 0;
    for (auto j = 0U; j < _entries[i].size(); j++) {
      _avgTexLength[i][j] = 0.0;
      int32_t rr          = 0;
      for (auto k = 0U; k < _entries[i][j].size(); k++) {
        _avgSizes[i] += _entries[i][j][k].getTexSize();
        _avgTexLength[i][j] += _entries[i][j][k].getTexLength();
        ar++;
        rr++;
      }
      _avgTexLength[i][j] /= rr;
    }
    _avgSizes[i] /= ar;
  }
}  // namespace painty

}  // namespace painty
