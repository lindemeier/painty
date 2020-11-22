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
#include "prgl/Texture2d.hxx"

namespace painty {

class TextureBrushDictionary {
 public:
  struct Entry {
    Mat1d texHost;
    std::shared_ptr<prgl::Texture2d> texGpu;
  };

 private:
  auto loadHeightMap(const std::string& file) const -> Mat1d;

  void createBrushTexturesFromFolder(const std::string& textureListFile);

 public:
  TextureBrushDictionary();

  auto lookup(const std::vector<vec2>& path, const double brushSize) const
    -> Entry;

 private:
  std::vector<std::vector<std::vector<Entry>>> _brushTexturesBySizeByLength;
  std::vector<double> _avgSizes;
  std::vector<std::vector<double>> _avgTexLength;
};

}  // namespace painty
