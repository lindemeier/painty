/**
 * @file image_io.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */
#ifndef PAINTY_IMAGE_IO_H
#define PAINTY_IMAGE_IO_H

#include <painty/core/vec.h>
#include <painty/image/mat.h>

namespace painty {
namespace io {
enum class ChannelDepth : uint8_t { BITS_8 = 0U, BITS_16 = 1U };

bool imRead(const std::string& filename, Mat<vec3>& linear_rgb);

bool imRead(const std::string& filename, Mat<double>& gray);

bool imSave(const std::string& filename, const Mat<vec3>& linear_rgb,
            const ChannelDepth bit_depth = ChannelDepth::BITS_16);

bool imSave(const std::string& filename, const Mat<double>& gray,
            const ChannelDepth bit_depth = ChannelDepth::BITS_16);
}  // namespace io

}  // namespace painty

#endif  // PAINTY_IMAGE_IO_H
