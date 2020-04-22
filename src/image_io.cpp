#include "painty/image_io.h"

#include <string>

#include <png.h>

bool painty::io::imRead(const std::string& filename, Mat<vec3>& srgb)
{
  FILE* fp = fopen(filename.c_str(), "rb");

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png)
    abort();

  png_infop info = png_create_info_struct(png);
  if (!info)
  {
    throw std::runtime_error("could not create png info struct");
  }

  return true;
}

bool painty::io::imRead(const std::string& filename, Mat<double>& gray)
{
}

bool painty::io::imSave(const std::string& filename, const Mat<vec3>& srgb)
{
}

bool painty::io::imSave(const std::string& filename, const Mat<double>& gray)
{
}
