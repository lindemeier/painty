#include "painty/io/image_io.h"

#include <ColorConverter/ColorConverter.hxx>
#include <algorithm>
#include <cstring>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <string>

static std::string extractFiletype(const std::string& filename) {
  std::string res(filename);
  size_t ls = res.find_last_of(".");
  res       = res.substr(ls + 1, res.size() - ls - 1);
  return res;
}

void painty::io::imRead(const std::string& filenameOriginal,
                        Mat<vec3>& linear_rgb) {
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

  // convert from sRGB to linear rgb
  color::ColorConverter<double, vec3> converter;
  for (auto v : linear_rgb) {
    converter.srgb2rgb(v, v);
  }
}

void painty::io::imRead(const std::string& filenameOriginal,
                        Mat<double>& gray) {
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
}

bool painty::io::imSave(const std::string& filenameOriginal,
                        const Mat<vec3>& linear_rgb) {
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  std::string filetype = extractFiletype(filename);

  // convert from linear rgb to srgb
  color::ColorConverter<double, vec3> converter;
  Mat<vec3> srgb(linear_rgb.size());
  for (int32_t i = 0; i < static_cast<int32_t>(linear_rgb.total()); i++) {
    converter.rgb2srgb(linear_rgb(i), srgb(i));
  }

  cv::Mat m;
  if (filetype == "png") {
    const auto scale = static_cast<double>(0xffff);
    srgb.convertTo(m, CV_MAKETYPE(CV_16U, 3), scale);
  } else {
    const auto scale = static_cast<double>(0xff);
    srgb.convertTo(m, CV_MAKETYPE(CV_8U, 3), scale);
  }
  cv::cvtColor(m, m, cv::COLOR_RGB2BGR);
  std::vector<int32_t> params;
  params.push_back(cv::IMWRITE_JPEG_QUALITY);
  params.push_back(100);
  params.push_back(cv::IMWRITE_PNG_COMPRESSION);
  params.push_back(0);
  return cv::imwrite(filename, m, params);
}

bool painty::io::imSave(const std::string& filenameOriginal,
                        const Mat<double>& gray) {
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  std::string filetype = extractFiletype(filename);

  cv::Mat m;
  if (filetype == "png") {
    const auto scale = static_cast<double>(0xffff);
    gray.convertTo(m, CV_MAKETYPE(CV_16U, 1), scale);
  } else {
    const auto scale = static_cast<double>(0xff);
    gray.convertTo(m, CV_MAKETYPE(CV_8U, 1), scale);
  }
  cv::cvtColor(m, m, cv::COLOR_RGB2BGR);
  std::vector<int32_t> params;
  params.push_back(cv::IMWRITE_JPEG_QUALITY);
  params.push_back(100);
  params.push_back(cv::IMWRITE_PNG_COMPRESSION);
  params.push_back(0);
  return cv::imwrite(filename, m, params);
}
