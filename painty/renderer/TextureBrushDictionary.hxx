/**
 * @file TextureBrushDictionary.hxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-09-29
 *
 */
#pragma once

#include <vector>

#include "painty/image/Mat.hxx"

namespace painty {
struct TextureBrushDictionaryEntry {
  int32_t id;
  int32_t radiusNr;
  int32_t lengthNr;

  Mat1d texture;

  int32_t getTexSize() const;
  int32_t getTexLength() const;
};

class TextureBrushDictionary {
 private:
  auto loadHeightMap(const std::vector<uint8_t>& data) const -> Mat1d;
  auto loadHeightMap(const std::string& file) const -> Mat1d;

  void createBrushTexturesFromFolder(const std::string& textureListFile);

 public:
  TextureBrushDictionary();

  auto lookup(const std::vector<vec2>& path, const double brushSize) const
    -> Mat1d;

 private:
  std::vector<std::vector<std::vector<TextureBrushDictionaryEntry> > > _entries;
  std::vector<double> _avgSizes;
  std::vector<std::vector<double> > _avgTexLength;
};

}  // namespace painty
