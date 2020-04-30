#include "painty/image_io.h"

#include <algorithm>
#include <cstring>
#include <string>

#include <png.h>
#include <ColorConverter/ColorConverter.hxx>

bool painty::io::imRead(const std::string& filename, Mat<vec3>& linear_rgb)
{
  png_image png_image; /* The control structure used by libpng */
                       /* Initialize the 'png_image' structure. */
  std::memset(&png_image, 0, (sizeof png_image));
  png_image.version = PNG_IMAGE_VERSION;

  linear_rgb = Mat<vec3>();

  auto success = false;

  if (png_image_begin_read_from_file(&png_image, filename.c_str()) != 0)
  {
    png_bytep buffer = nullptr;

    // PNG_FORMAT_FLAG_LINEAR | PNG_FORMAT_FLAG_COLOR
    // 3 channel 16bit depth
    png_image.format = PNG_FORMAT_LINEAR_RGB;
    png_image.flags = PNG_IMAGE_FLAG_COLORSPACE_NOT_sRGB;

    buffer = static_cast<png_bytep>(std::malloc(PNG_IMAGE_SIZE(png_image)));

    if (buffer != nullptr && png_image_finish_read(&png_image, nullptr, buffer, 0, nullptr) != 0)
    {
      // process
      linear_rgb = painty::Mat<vec3>(png_image.height, png_image.width);

      constexpr auto scale = 1.0 / static_cast<double>(0xFFFF);
      auto& data = linear_rgb.getData();
      for (auto i = 0U; i < data.size(); i++)
      {
        auto px_ptr = reinterpret_cast<png_uint_16p>(&(buffer[i * 3U * 2U]));

        data[i][0] = static_cast<double>(px_ptr[0]) * scale;
        data[i][1] = static_cast<double>(px_ptr[1]) * scale;
        data[i][2] = static_cast<double>(px_ptr[2]) * scale;
      }
      success = true;
    }

    png_image_free(&png_image);
    if (buffer)
    {
      free(buffer);
    }
  }
  return success;
}

bool painty::io::imRead(const std::string& filename, Mat<double>& gray)
{
  png_image png_image;

  std::memset(&png_image, 0, (sizeof png_image));
  png_image.version = PNG_IMAGE_VERSION;

  gray = Mat<double>();

  auto success = false;

  if (png_image_begin_read_from_file(&png_image, filename.c_str()) != 0)
  {
    png_bytep buffer = nullptr;

    // 1 channel 16bit depth
    png_image.format = PNG_FORMAT_LINEAR_Y;
    png_image.flags = PNG_IMAGE_FLAG_COLORSPACE_NOT_sRGB;

    buffer = static_cast<png_bytep>(std::malloc(PNG_IMAGE_SIZE(png_image)));

    if (buffer != nullptr && png_image_finish_read(&png_image, nullptr, buffer, 0, nullptr) != 0)
    {
      // process
      gray = painty::Mat<double>(png_image.height, png_image.width);

      constexpr auto scale = 1.0 / static_cast<double>(0xFFFF);
      auto& data = gray.getData();
      for (auto i = 0U; i < data.size(); i++)
      {
        auto px_ptr = reinterpret_cast<png_uint_16p>(&(buffer[i * 2U]));

        data[i] = static_cast<double>(px_ptr[0]) * scale;
      }
      success = true;
    }

    png_image_free(&png_image);
    if (buffer)
    {
      free(buffer);
    }
  }
  return success;
}

bool painty::io::imSave(const std::string& filename, const Mat<vec3>& linear_rgb, const ChannelDepth bit_depth)
{
  auto success = false;

  png_image png_image;
  memset(&png_image, 0, sizeof(png_image));
  png_image.version = PNG_IMAGE_VERSION;

  png_image.width = linear_rgb.getCols();
  png_image.height = linear_rgb.getRows();
  png_image.format = (bit_depth == ChannelDepth::BITS_16) ? PNG_FORMAT_LINEAR_RGB : PNG_FORMAT_RGB;
  png_image.flags = PNG_IMAGE_FLAG_COLORSPACE_NOT_sRGB;

  const auto& image_data = linear_rgb.getData();

  if (bit_depth == ChannelDepth::BITS_16)
  {
    std::vector<png_uint_16> bit16_data(image_data.size() * 3U);
    constexpr auto scale = static_cast<double>(0xFFFF);
    for (auto i = 0U; i < image_data.size(); i++)
    {
      auto px_ptr = reinterpret_cast<png_uint_16p>(&(bit16_data[i * 3U]));

      px_ptr[0] = static_cast<png_uint_16>(std::clamp(image_data[i][0] * scale, 0.0, scale));
      px_ptr[1] = static_cast<png_uint_16>(std::clamp(image_data[i][1] * scale, 0.0, scale));
      px_ptr[2] = static_cast<png_uint_16>(std::clamp(image_data[i][2] * scale, 0.0, scale));
    }

    success = png_image_write_to_file(&png_image, filename.c_str(), 0, bit16_data.data(), 0, NULL);
  }
  else
  {
    throw std::runtime_error("8bit saving currently not supported");

    std::vector<png_byte> bit8_data(image_data.size() * 3U);
    constexpr auto scale = static_cast<double>(0xFF);
    for (auto i = 0U; i < image_data.size(); i++)
    {
      auto px_ptr = reinterpret_cast<png_bytep>(&(bit8_data[i * 3U]));

      px_ptr[0] = static_cast<png_byte>(std::clamp(image_data[i][0] * scale, 0.0, scale));
      px_ptr[1] = static_cast<png_byte>(std::clamp(image_data[i][1] * scale, 0.0, scale));
      px_ptr[2] = static_cast<png_byte>(std::clamp(image_data[i][2] * scale, 0.0, scale));
    }
    success = png_image_write_to_file(&png_image, filename.c_str(), 0, bit8_data.data(), 0, NULL);
  }

  return success;
}

bool painty::io::imSave(const std::string& filename, const Mat<double>& gray, const ChannelDepth bit_depth)
{
  auto success = false;

  png_image png_image;
  memset(&png_image, 0, sizeof(png_image));
  png_image.version = PNG_IMAGE_VERSION;

  png_image.width = gray.getCols();
  png_image.height = gray.getRows();
  png_image.format = (bit_depth == ChannelDepth::BITS_16) ? PNG_FORMAT_LINEAR_Y : PNG_FORMAT_GRAY;
  png_image.flags = PNG_IMAGE_FLAG_COLORSPACE_NOT_sRGB;

  const auto& image_data = gray.getData();
  if (bit_depth == ChannelDepth::BITS_16)
  {
    std::vector<png_uint_16> bit16_data(image_data.size());
    constexpr auto scale = static_cast<double>(0xFFFF);
    for (auto i = 0U; i < image_data.size(); i++)
    {
      auto px_ptr = reinterpret_cast<png_uint_16p>(&(bit16_data[i]));
      px_ptr[0] = static_cast<png_uint_16>(std::clamp(image_data[i] * scale, 0.0, scale));
    }
    success = png_image_write_to_file(&png_image, filename.c_str(), 0, bit16_data.data(), 0, NULL);
  }
  else
  {
    std::vector<png_byte> bit8_data(image_data.size());
    constexpr auto scale = static_cast<double>(0xFF);
    for (auto i = 0U; i < image_data.size(); i++)
    {
      auto px_ptr = reinterpret_cast<png_bytep>(&(bit8_data[i]));
      px_ptr[0] = static_cast<png_byte>(std::clamp(image_data[i] * scale, 0.0, scale));
    }
    success = png_image_write_to_file(&png_image, filename.c_str(), 0, bit8_data.data(), 0, NULL);
  }

  return success;
}
