/**
 * @file image_io.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */
#ifndef PAINTY_IMAGE_IO_H
#define PAINTY_IMAGE_IO_H

#include <painty/mat.h>
#include <painty/vec.h>

namespace painty
{
namespace io
{
void imRead(const std::string& filename, Mat<vec3>& linear_rgb);

void imRead(const std::string& filename, Mat<double>& gray);

void imSave(const std::string& filename, const Mat<vec3>& srgb);

void imSave(const std::string& filename, const Mat<double>& gray);
}  // namespace io

}  // namespace painty

#endif  // PAINTY_IMAGE_IO_H
