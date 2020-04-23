#include "painty/image_io.h"

#include <string>

#include <png.h>

void painty::io::imRead(const std::string& filename, Mat<vec3>& srgb)
{
  FILE* fp = fopen(filename.c_str(), "rb");

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png)
  {
    png_destroy_read_struct(&png, NULL, NULL);
    fclose(fp);
    throw std::runtime_error("could not create png read struct");
  }

  // png->format = PNG_FORMAT_FLAG_COLOR | PNG_FORMAT_FLAG_LINEAR;

  png_infop info = png_create_info_struct(png);
  if (!info)
  {
    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);
    throw std::runtime_error("could not create png info struct");
  }

  if (setjmp(png_jmpbuf(png)))
  {
    fclose(fp);
    throw std::runtime_error("could not set jump png");
  }

  png_init_io(png, fp);

  png_read_info(png, info);

  const auto width = png_get_image_width(png, info);
  const auto height = png_get_image_height(png, info);
  const auto color_type = png_get_color_type(png, info);
  const auto bit_depth = png_get_bit_depth(png, info);

  constexpr auto BIT_DEPTH_16 = static_cast<png_byte>(16U);
  if (bit_depth == BIT_DEPTH_16)
  {
    // png_set_strip_16(png);
  }

  if (color_type == PNG_COLOR_TYPE_PALETTE)
  {
    png_set_palette_to_rgb(png);
  }

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
  constexpr auto BIT_DEPTH_8 = static_cast<png_byte>(8U);
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < BIT_DEPTH_8)
  {
    png_set_expand_gray_1_2_4_to_8(png);
  }

  if (png_get_valid(png, info, PNG_INFO_tRNS))
  {
    png_set_tRNS_to_alpha(png);
  }

  // These color_type don't have an alpha channel then fill it with 0xff.
  if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
  {
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
  }

  if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
  {
    png_set_gray_to_rgb(png);
  }

  png_read_update_info(png, info);

  auto row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for (png_uint_32 y = 0U; y < height; y++)
  {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
  }

  png_read_image(png, row_pointers);
  fclose(fp);
  png_destroy_read_struct(&png, &info, NULL);

  // copy in image
  srgb = Mat<vec3>(height, width);
  const auto scale = 1.0 / (std::pow(2.0, bit_depth) - 1.0);
  for (png_uint_32 y = 0; y < height; y++)
  {
    png_bytep row = row_pointers[y];
    for (png_uint_32 x = 0; x < width; x++)
    {
      png_bytep px = &(row[x * 4]);
      // skip alpha at index 3
      srgb(y, x)[0] = static_cast<double>(px[0]) * scale;
      srgb(y, x)[1] = static_cast<double>(px[1]) * scale;
      srgb(y, x)[2] = static_cast<double>(px[2]) * scale;
    }
  }

  // free allocated memory
  for (png_uint_32 y = 0U; y < height; y++)
  {
    free(row_pointers[y]);
  }
  free(row_pointers);
}

void painty::io::imRead(const std::string& filename, Mat<double>& gray)
{
}

void painty::io::imSave(const std::string& filename, const Mat<vec3>& srgb)
{
}

void painty::io::imSave(const std::string& filename, const Mat<double>& gray)
{
}
