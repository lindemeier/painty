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

class TextureBrushDictionary {
 private:
  auto loadHeightMap(const std::string& file) const -> Mat1d;

  void createBrushTexturesFromFolder(const std::string& textureListFile);

 public:
  TextureBrushDictionary();

  auto lookup(const std::vector<vec2>& path, const double brushSize) const
    -> Mat1d;

 private:
  std::vector<std::vector<std::vector<Mat1d>>> _brushTexturesBySizeByLength;
  std::vector<double> _avgSizes;
  std::vector<std::vector<double>> _avgTexLength;
};

}  // namespace painty
