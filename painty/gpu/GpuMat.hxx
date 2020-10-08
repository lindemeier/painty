/**
 * @file GpuMat.hxx
 * @author thomas lindemeier
 * @brief
 * @date 2020-10-08
 *
 */
#pragma once

#include "painty/core/Types.hxx"
#include "painty/image/Mat.hxx"
#include "prgl/Texture2d.hxx"

namespace painty {

template <class T>
class GpuMat final {
 public:
  GpuMat(const Size& size) : _size(size) {}
  GpuMat(const Mat<T>& cpuImage)
      : _size({static_cast<uint32_t>(cpuImage.cols),
               static_cast<uint32_t>(cpuImage.rows)}) {
    upload(cpuImage);
  }

  void upload(const Mat<T>& m) {
    _mat = m;

    if ((_texture == nullptr) ||
        (static_cast<uint32_t>(_mat.cols) != _size.width) ||
        (static_cast<uint32_t>(_mat.rows) != _size.height)) {
      const auto type =
        prgl::DataTypeTr<typename DataType<T>::channel_type>::dataType;
      const auto nrChannels = DataType<T>::dim;

      auto internalFormat = prgl::TextureFormatInternal::Rgb32F;
      auto format         = prgl::TextureFormat::Rgb;
      switch (type) {
        case prgl::DataType::UnsignedByte: {
          if (nrChannels == 1U) {
            format         = prgl::TextureFormat::Red;
            internalFormat = prgl::TextureFormatInternal::R8Ui;
          } else if (nrChannels == 2U) {
            format         = prgl::TextureFormat::Rg;
            internalFormat = prgl::TextureFormatInternal::Rg8Ui;
          } else if (nrChannels == 3U) {
            format         = prgl::TextureFormat::Rgb;
            internalFormat = prgl::TextureFormatInternal::Rgb8Ui;
          } else if (nrChannels == 4U) {
            format         = prgl::TextureFormat::Rgba;
            internalFormat = prgl::TextureFormatInternal::Rgba8Ui;
          }
          break;
        }
        case prgl::DataType::UnsignedShort: {
          if (nrChannels == 1U) {
            format         = prgl::TextureFormat::Red;
            internalFormat = prgl::TextureFormatInternal::R16Ui;
          } else if (nrChannels == 2U) {
            format         = prgl::TextureFormat::Rg;
            internalFormat = prgl::TextureFormatInternal::Rg16Ui;
          } else if (nrChannels == 3U) {
            format         = prgl::TextureFormat::Rgb;
            internalFormat = prgl::TextureFormatInternal::Rgb16Ui;
          } else if (nrChannels == 4U) {
            format         = prgl::TextureFormat::Rgba;
            internalFormat = prgl::TextureFormatInternal::Rgba16Ui;
          }
          break;
        }
        case prgl::DataType::Float:
        case prgl::DataType::Double: {
          if (nrChannels == 1U) {
            format         = prgl::TextureFormat::Red;
            internalFormat = prgl::TextureFormatInternal::R32F;
          } else if (nrChannels == 2U) {
            format         = prgl::TextureFormat::Rg;
            internalFormat = prgl::TextureFormatInternal::Rg32F;
          } else if (nrChannels == 3U) {
            format         = prgl::TextureFormat::Rgb;
            internalFormat = prgl::TextureFormatInternal::Rgb32F;
          } else if (nrChannels == 4U) {
            format         = prgl::TextureFormat::Rgba;
            internalFormat = prgl::TextureFormatInternal::Rgba32F;
          }
          break;
        }
        case prgl::DataType::Byte:
        case prgl::DataType::Short:
        case prgl::DataType::UnsignedInt:
        case prgl::DataType::Int:
        case prgl::DataType::HalfFloat: {
          throw std::invalid_argument("Mat type not supported by GpuMat.");
          break;
        }
      }

      const auto minFilter = prgl::TextureMinFilter::Linear;
      const auto magFilter = prgl::TextureMagFilter::Linear;
      const auto envMode   = prgl::TextureEnvMode::Replace;
      const auto wrapMode  = prgl::TextureWrapMode::Repeat;

      _texture = prgl::Texture2d::Create(
        _size.width, _size.height, internalFormat, format, type, minFilter,
        magFilter, envMode, wrapMode, false);
    }

    Mat<T> yFlipped(_mat.size());
    cv::flip(_mat, yFlipped, 0);

    _texture->upload(yFlipped.data);
  }

  auto getMat() const -> const Mat<T>& {
    return _mat;
  }

  auto getSize() const -> const Size& {
    return _size;
  }

  auto getTexture() -> const std::shared_ptr<prgl::Texture2d>& {
    return _texture;
  }

  void download() {
    const auto type =
      prgl::DataTypeTr<typename DataType<T>::channel_type>::dataType;
    const auto nrChannels = DataType<T>::dim;

    if ((static_cast<uint32_t>(_mat.cols) != _size.width) ||
        (static_cast<uint32_t>(_mat.rows) != _size.height)) {
      _mat = Mat<T>(static_cast<int32_t>(_size.height),
                    static_cast<int32_t>(_size.width));
    }
    auto format = prgl::TextureFormat::Red;
    if (nrChannels == 1U) {
      format = prgl::TextureFormat::Red;
    } else if (nrChannels == 2U) {
      format = prgl::TextureFormat::Rg;
    } else if (nrChannels == 3U) {
      format = prgl::TextureFormat::Rgb;
    } else if (nrChannels == 4U) {
      format = prgl::TextureFormat::Rgba;
    }
    _texture->download(&(_mat.data), format, type);

    cv::flip(_mat, _mat, 0);
  }

 private:
  Size _size;

  std::shared_ptr<prgl::Texture2d> _texture = nullptr;

  Mat<T> _mat = {};
};  // namespace painty
}  // namespace painty
