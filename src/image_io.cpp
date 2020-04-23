#include "painty/image_io.h"

#include <string>
#include <cstring>

#include <png.h>

void painty::io::imRead(const std::string& filename, Mat<vec3>& linear_rgb)
{
  png_image png_image; /* The control structure used by libpng */
                       /* Initialize the 'png_image' structure. */
  std::memset(&png_image, 0, (sizeof png_image));
  png_image.version = PNG_IMAGE_VERSION;

  linear_rgb = Mat<vec3>();

  if (png_image_begin_read_from_file(&png_image, filename.c_str()) != 0)
  {
    png_bytep buffer = nullptr;

    // PNG_FORMAT_FLAG_LINEAR | PNG_FORMAT_FLAG_COLOR
    // 3 channel 16bit depth
    png_image.format = PNG_FORMAT_LINEAR_RGB;

    buffer = static_cast<png_bytep>(std::malloc(PNG_IMAGE_SIZE(png_image)));

    if (buffer != nullptr && png_image_finish_read(&png_image, nullptr, buffer, 0, nullptr) != 0)
    {
      // process
      linear_rgb = painty::Mat<vec3>(png_image.height, png_image.width);

      constexpr auto scale = 1.0 / static_cast<double>(0xFFFF);
      auto& data = linear_rgb.getData();
      for (auto i = 0U; i < data.size(); i++)
      {
        auto px = reinterpret_cast<png_uint_16p>(&(buffer[i * 3U * 2U]));

        data[i][0] = static_cast<double>(px[0]) * scale;
        data[i][1] = static_cast<double>(px[1]) * scale;
        data[i][2] = static_cast<double>(px[2]) * scale;
      }
    }

    png_image_free(&png_image);
    if (buffer)
    {
      free(buffer);
    }
  }
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
