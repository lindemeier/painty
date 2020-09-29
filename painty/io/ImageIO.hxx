/**
 * @file ImageIO.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */
#pragma once

#include "painty/core/Vec.hxx"
#include "painty/image/Mat.hxx"

namespace painty {
namespace io {
void imRead(const std::string& filename, Mat<vec3>& linear_rgb,
            bool convertFrom_sRGB);

void imRead(const std::string& filename, Mat<double>& gray,
            bool convertFrom_sRGB);

void imRead(const std::vector<uint8_t>& buffer, Mat<double>& gray,
            bool convertFrom_sRGB);

bool imSave(const std::string& filename, const Mat<vec3>& linear_rgb,
            bool convertTo_sRGB);

bool imSave(const std::string& filename, const Mat<double>& gray,
            bool convertTo_sRGB);

bool imSave(std::vector<uint8_t>& buffer, const Mat<double>& gray,
            bool convertTo_sRGB);
}  // namespace io

}  // namespace painty
