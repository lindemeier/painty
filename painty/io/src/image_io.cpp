#include "painty/io/image_io.h"

#include <algorithm>
#include <cstring>
#include <string>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "painty/core/color.h"

static std::string extractFiletype(const std::string& filename) {
  std::string res(filename);
  size_t ls = res.find_last_of(".");
  res       = res.substr(ls + 1, res.size() - ls - 1);
  return res;
}

/**
 * @brief Reads images from files.
 *
 * @param filenameOriginal
 * @param linear_rgb
 * @param convertFrom_sRGB linearize rgb values if true
 */
void painty::io::imRead(const std::string& filenameOriginal,
                        Mat<vec3>& linear_rgb, const bool convertFrom_sRGB) {
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  cv::Mat cv_mat = cv::imread(filename, cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);

  // if not loaded succesfully
  if (!cv_mat.data) {
    throw std::ios_base::failure(filenameOriginal);
  }

  if (cv_mat.channels() == 1) {
    cv::Mat in[] = {cv_mat, cv_mat, cv_mat};
    cv::merge(in, 3, cv_mat);
  } else if (cv_mat.channels() == 4) {
    cv::cvtColor(cv_mat, cv_mat, cv::COLOR_BGRA2BGR);
  }

  // data scale
  double scale = 1.0;
  if (cv_mat.depth() == CV_16U)
    scale = 1.0 / (0xffff);
  else if (cv_mat.depth() == CV_32F)
    scale = 1.0;
  else if (cv_mat.depth() == CV_8U)
    scale = 1.0 / (0xff);
  else if (cv_mat.depth() == CV_64F)
    scale = 1.0 / (0xffffffff);

  // OpenCV has BGR
  cv::cvtColor(cv_mat, cv_mat, cv::COLOR_BGR2RGB);

  // convert to right type
  cv_mat.convertTo(linear_rgb, CV_64FC3, scale);

  if (convertFrom_sRGB) {
    // convert from sRGB to linear rgb
    ColorConverter<double> converter;
    for (auto& v : linear_rgb) {
      converter.srgb2rgb(v, v);
    }
  }
}

/**
 * @brief Reads grayscale images from file.
 *
 * @param filenameOriginal
 * @param gray
 * @param convertFrom_sRGB linearize values if true
 */
void painty::io::imRead(const std::string& filenameOriginal, Mat<double>& gray,
                        const bool convertFrom_sRGB) {
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  cv::Mat cv_mat =
    cv::imread(filename, cv::IMREAD_ANYDEPTH | cv::IMREAD_GRAYSCALE);

  // if not loaded succesfully
  if (!cv_mat.data) {
    throw std::ios_base::failure(filenameOriginal);
  }

  // data scale
  double scale = 1.0;
  if (cv_mat.depth() == CV_16U)
    scale = 1.0 / 0xffff;
  else if (cv_mat.depth() == CV_32F)
    scale = 1.0;
  else if (cv_mat.depth() == CV_8U)
    scale = 1.0 / 0xff;
  else if (cv_mat.depth() == CV_64F)
    scale = 1.0;

  // convert to right type
  cv_mat.convertTo(gray, CV_64FC1, scale);

  if (convertFrom_sRGB) {
    // convert from sRGB to linear rgb
    ColorConverter<double> converter;
    for (auto& v : gray) {
      converter.srgb2rgb(v, v);
    }
  }
}

/**
 * @brief Write images to file.
 *
 * @param filenameOriginal
 * @param linear_rgb
 * @param convertTo_sRGB convert image to sRGB when saving.
 * @return true
 * @return false
 */
bool painty::io::imSave(const std::string& filenameOriginal,
                        const Mat<vec3>& linear_rgb,
                        const bool convertTo_sRGB) {
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  std::string filetype = extractFiletype(filename);

  // convert from linear rgb to srgb
  ColorConverter<double> converter;
  Mat<vec3> out(linear_rgb.size());
  if (convertTo_sRGB) {
    for (int32_t i = 0; i < static_cast<int32_t>(linear_rgb.total()); i++) {
      converter.rgb2srgb(linear_rgb(i), out(i));
    }
  } else {
    out = linear_rgb;
  }

  cv::Mat m;
  if (filetype == "png") {
    const auto scale = static_cast<double>(0xffff);
    out.convertTo(m, CV_MAKETYPE(CV_16U, 3), scale);
  } else {
    const auto scale = static_cast<double>(0xff);
    out.convertTo(m, CV_MAKETYPE(CV_8U, 3), scale);
  }
  cv::cvtColor(m, m, cv::COLOR_RGB2BGR);
  std::vector<int32_t> params;
  params.push_back(cv::IMWRITE_JPEG_QUALITY);
  params.push_back(100);
  params.push_back(cv::IMWRITE_PNG_COMPRESSION);
  params.push_back(0);
  return cv::imwrite(filename, m, params);
}

/**
 * @brief Save images to a file.
 *
 * @param filenameOriginal
 * @param gray
 * @param convertTo_sRGB whether to convert to sRGB.
 * @return true
 * @return false
 */
bool painty::io::imSave(const std::string& filenameOriginal,
                        const Mat<double>& gray, const bool convertTo_sRGB) {
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  std::string filetype = extractFiletype(filename);

  ColorConverter<double> converter;
  Mat<double> out(gray.size());
  if (convertTo_sRGB) {
    for (int32_t i = 0; i < static_cast<int32_t>(gray.total()); i++) {
      converter.rgb2srgb(gray(i), out(i));
    }
  } else {
    out = gray;
  }

  cv::Mat m;
  if (filetype == "png") {
    const auto scale = static_cast<double>(0xffff);
    out.convertTo(m, CV_MAKETYPE(CV_16U, 1), scale);
  } else {
    const auto scale = static_cast<double>(0xff);
    out.convertTo(m, CV_MAKETYPE(CV_8U, 1), scale);
  }
  std::vector<int32_t> params;
  params.push_back(cv::IMWRITE_JPEG_QUALITY);
  params.push_back(100);
  params.push_back(cv::IMWRITE_PNG_COMPRESSION);
  params.push_back(0);
  return cv::imwrite(filename, m, params);
}
