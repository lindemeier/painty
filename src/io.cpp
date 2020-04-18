#include "painty/io.h"

#include <string>
#include <vector>

#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace painty
{
constexpr auto U8_MAX = 0xFF;
constexpr auto U16_MAX = 0xFFFF;
constexpr auto F32_MAX = 1.0F;
constexpr auto F64_MAX = 1.0;

static std::string extractFiletype(const std::string& filename)
{
  std::string res(filename);
  size_t ls = res.find_last_of(".");
  res = res.substr(ls + 1, res.size() - ls - 1);
  return res;
}

void imLoad(const std::string& filenameOriginal, Mat4f& image)
{
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  cv::Mat cv_mat = cv::imread(filename, cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);

  // if not loaded succesfully
  if (!cv_mat.data)
  {
    throw std::ios_base::failure(filenameOriginal);
  }

  if (cv_mat.channels() == 1)
  {
    cv::Mat in[] = { cv_mat, cv_mat, cv_mat, cv_mat };
    cv::merge(in, 4, cv_mat);
  }

  // data scale
  double scale = F32_MAX;
  if (cv_mat.depth() == CV_16U)
    scale = 1.0 / U16_MAX;
  else if (cv_mat.depth() == CV_32F)
    scale = F32_MAX;
  else if (cv_mat.depth() == CV_8U)
    scale = 1.0 / U8_MAX;
  else if (cv_mat.depth() == CV_64F)
    scale = F64_MAX;

  // convert to right type
  cv_mat.convertTo(cv_mat, CV_32FC4, scale);

  // OpenCV has BGR
  cv::cvtColor(cv_mat, image, cv::COLOR_BGRA2RGBA);
}

void imLoad(const std::string& filenameOriginal, Mat3f& image)
{
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  cv::Mat cv_mat = cv::imread(filename, cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);

  // if not loaded succesfully
  if (!cv_mat.data)
  {
    throw std::ios_base::failure(filenameOriginal);
  }

  if (cv_mat.channels() == 1)
  {
    cv::Mat in[] = { cv_mat, cv_mat, cv_mat };
    cv::merge(in, 3, cv_mat);
  }
  else if (cv_mat.channels() == 4)
  {
    cv::cvtColor(cv_mat, cv_mat, cv::COLOR_BGRA2BGR);
  }

  // data scale
  double scale = F32_MAX;
  if (cv_mat.depth() == CV_16U)
    scale = 1.0 / U16_MAX;
  else if (cv_mat.depth() == CV_32F)
    scale = F32_MAX;
  else if (cv_mat.depth() == CV_8U)
    scale = 1.0 / U8_MAX;
  else if (cv_mat.depth() == CV_64F)
    scale = F64_MAX;

  // convert to right type
  cv_mat.convertTo(cv_mat, CV_32FC3, scale);

  // OpenCV has BGR
  cv::cvtColor(cv_mat, image, cv::COLOR_BGR2RGB);
}

void imLoad(const std::string& filenameOriginal, Mat1f& image)
{
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  cv::Mat cv_mat = cv::imread(filename, cv::IMREAD_ANYDEPTH | cv::IMREAD_GRAYSCALE);

  // if not loaded succesfully
  if (!cv_mat.data)
  {
    throw std::ios_base::failure(filenameOriginal);
  }

  // data scale
  double scale = F32_MAX;
  if (cv_mat.depth() == CV_16U)
    scale = 1.0 / U16_MAX;
  else if (cv_mat.depth() == CV_32F)
    scale = F32_MAX;
  else if (cv_mat.depth() == CV_8U)
    scale = 1.0 / U8_MAX;
  else if (cv_mat.depth() == CV_64F)
    scale = F64_MAX;

  // convert to right type
  cv_mat.convertTo(image, CV_32FC1, scale);
}

void imLoad(const std::string& filenameOriginal, Mat1d& image)
{
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  cv::Mat cv_mat = cv::imread(filename, cv::IMREAD_ANYDEPTH | cv::IMREAD_GRAYSCALE);

  // if not loaded succesfully
  if (!cv_mat.data)
  {
    throw std::ios_base::failure(filenameOriginal);
  }

  // data scale
  double scale = F32_MAX;
  if (cv_mat.depth() == CV_16U)
    scale = 1.0 / U16_MAX;
  else if (cv_mat.depth() == CV_32F)
    scale = F32_MAX;
  else if (cv_mat.depth() == CV_8U)
    scale = 1.0 / U8_MAX;
  else if (cv_mat.depth() == CV_64F)
    scale = F32_MAX;

  // convert to right type
  cv_mat.convertTo(image, CV_64FC1, scale);
}

void imLoad(const std::string& filenameOriginal, Mat1u& image)
{
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  cv::Mat cv_mat = cv::imread(filename, cv::IMREAD_ANYDEPTH | cv::IMREAD_GRAYSCALE);

  // if not loaded succesfully
  if (!cv_mat.data)
  {
    throw std::ios_base::failure(filenameOriginal);
  }

  // data scale
  double scale = F32_MAX;
  if (cv_mat.depth() == CV_16U)
    scale = 1.0 / U8_MAX;
  else if (cv_mat.depth() == CV_32F)
    scale = 255.f;
  else if (cv_mat.depth() == CV_8U)
    scale = 1.f;

  // convert to right type
  cv_mat.convertTo(image, CV_8UC1, scale);
}

bool imSave(const std::string& filenameOriginal, const Mat3f& output)
{
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  std::string filetype = extractFiletype(filename);

  cv::Mat m;
  if (filetype == "png")
  {
    const double scale = (float)U16_MAX;
    output.convertTo(m, CV_MAKETYPE(CV_16U, 3), scale);
  }
  else
  {
    const double scale = (float)0xff;
    output.convertTo(m, CV_MAKETYPE(CV_8U, 3), scale);
  }
  cv::cvtColor(m, m, cv::COLOR_RGB2BGR);
  std::vector<int32_t> params;
  params.push_back(cv::IMWRITE_JPEG_QUALITY);
  params.push_back(100);
  params.push_back(cv::IMWRITE_PNG_COMPRESSION);
  params.push_back(0);
  return cv::imwrite(filename, m, params);
}

bool imSave(const std::string& filenameOriginal, const Mat4f& output)
{
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  std::string filetype = extractFiletype(filename);

  cv::Mat m;
  if (filetype == "png")
  {
    const double scale = (float)U16_MAX;
    output.convertTo(m, CV_MAKETYPE(CV_16U, 4), scale);
  }
  else
  {
    const double scale = (float)0xff;
    output.convertTo(m, CV_MAKETYPE(CV_8U, 4), scale);
  }
  cv::cvtColor(m, m, cv::COLOR_RGBA2BGRA);
  std::vector<int32_t> params;
  params.push_back(cv::IMWRITE_JPEG_QUALITY);
  params.push_back(100);
  params.push_back(cv::IMWRITE_PNG_COMPRESSION);
  params.push_back(0);
  return cv::imwrite(filename, m, params);
}

bool imSave(const std::string& filenameOriginal, const Mat1f& output)
{
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  std::string filetype = extractFiletype(filename);

  cv::Mat m;
  if (filetype == "png")
  {
    const double scale = (float)U16_MAX;
    output.convertTo(m, CV_MAKETYPE(CV_16U, 1), scale);
  }
  else
  {
    const double scale = (float)0xff;
    output.convertTo(m, CV_MAKETYPE(CV_8U, 1), scale);
  }
  std::vector<int32_t> params;
  params.push_back(cv::IMWRITE_JPEG_QUALITY);
  params.push_back(100);
  params.push_back(cv::IMWRITE_PNG_COMPRESSION);
  params.push_back(0);
  return cv::imwrite(filename, m, params);
}

bool imSave(const std::string& filenameOriginal, const Mat1d& output)
{
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  std::string filetype = extractFiletype(filename);

  cv::Mat m;
  if (filetype == "png")
  {
    const double scale = (double)U16_MAX;
    output.convertTo(m, CV_MAKETYPE(CV_16U, 1), scale);
  }
  else
  {
    const double scale = (double)0xff;
    output.convertTo(m, CV_MAKETYPE(CV_8U, 1), scale);
  }
  std::vector<int32_t> params;
  params.push_back(cv::IMWRITE_JPEG_QUALITY);
  params.push_back(100);
  params.push_back(cv::IMWRITE_PNG_COMPRESSION);
  params.push_back(0);
  return cv::imwrite(filename, m, params);
}

bool imSave(const std::string& filenameOriginal, const Mat1u& output)
{
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  std::string filetype = extractFiletype(filename);

  std::vector<int32_t> params;
  params.push_back(cv::IMWRITE_JPEG_QUALITY);
  params.push_back(100);
  params.push_back(cv::IMWRITE_PNG_COMPRESSION);
  params.push_back(0);
  return cv::imwrite(filename, output, params);
}

bool imSave(const std::string& filenameOriginal, const Mat3u& output)
{
  std::string filename = filenameOriginal;
  std::replace(filename.begin(), filename.end(), '\\', '/');

  std::string filetype = extractFiletype(filename);

  cv::cvtColor(output, output, cv::COLOR_RGB2BGR);
  std::vector<int32_t> params;
  params.push_back(cv::IMWRITE_JPEG_QUALITY);
  params.push_back(100);
  params.push_back(cv::IMWRITE_PNG_COMPRESSION);
  params.push_back(0);
  return cv::imwrite(filename, output, params);
}

void imDecode(const uchar* dataPtr, size_t size, Mat3f& image)
{
  cv::Mat buffer(size, 1, CV_8UC1, const_cast<uchar*>(dataPtr));
  cv::Mat cv_mat = cv::imdecode(buffer, cv::IMREAD_ANYDEPTH | cv::IMREAD_COLOR);

  // if not loaded succesfully
  if (!cv_mat.data)
  {
    throw std::ios_base::failure("decoding image");
  }

  if (cv_mat.channels() == 1)
  {
    cv::Mat in[] = { cv_mat, cv_mat, cv_mat };
    cv::merge(in, 3, cv_mat);
  }
  else if (cv_mat.channels() == 4)
  {
    cv::cvtColor(cv_mat, cv_mat, cv::COLOR_BGRA2BGR);
  }

  // data scale
  double scale = F32_MAX;
  if (cv_mat.depth() == CV_16U)
    scale = 1.0 / U16_MAX;
  else if (cv_mat.depth() == CV_32F)
    scale = F32_MAX;
  else if (cv_mat.depth() == CV_8U)
    scale = 1.0 / U8_MAX;
  else if (cv_mat.depth() == CV_64F)
    scale = F64_MAX;

  // convert to right type
  cv_mat.convertTo(cv_mat, CV_32FC3, scale);

  // OpenCV has BGR
  cv::cvtColor(cv_mat, image, cv::COLOR_BGR2RGB);
}

void imDecode(const uchar* dataPtr, size_t size, Mat1f& image)
{
  cv::Mat buffer(size, 1, CV_8UC1, const_cast<uchar*>(dataPtr));
  cv::Mat cv_mat = cv::imdecode(buffer, cv::IMREAD_ANYDEPTH | cv::IMREAD_GRAYSCALE);

  // if not loaded succesfully
  if (!cv_mat.data)
  {
    throw std::ios_base::failure("decoding image");
  }

  // data scale
  double scale = F32_MAX;
  if (cv_mat.depth() == CV_16U)
    scale = 1.0 / U16_MAX;
  else if (cv_mat.depth() == CV_32F)
    scale = F32_MAX;
  else if (cv_mat.depth() == CV_8U)
    scale = 1.0 / U8_MAX;
  else if (cv_mat.depth() == CV_64F)
    scale = F64_MAX;

  // convert to right type
  cv_mat.convertTo(image, CV_32FC1, scale);
}

void imDecode(const uchar* dataPtr, size_t size, Mat1u& image)
{
  cv::Mat buffer(size, 1, CV_8UC1, const_cast<uchar*>(dataPtr));
  cv::Mat cv_mat = cv::imdecode(buffer, cv::IMREAD_ANYDEPTH | cv::IMREAD_GRAYSCALE);

  // if not loaded succesfully
  if (!cv_mat.data)
  {
    throw std::ios_base::failure("decoding image");
  }

  // data scale
  double scale = F32_MAX;
  if (cv_mat.depth() == CV_16U)
    scale = 1.0 / U8_MAX;
  else if (cv_mat.depth() == CV_32F)
    scale = 255.f;
  else if (cv_mat.depth() == CV_8U)
    scale = 1.f;

  // convert to right type
  cv_mat.convertTo(image, CV_8UC1, scale);
}

bool imEncode(std::vector<uint8_t>& buffer, const std::string& type, const Mat3f& image)
{
  cv::Mat m;
  if (type == ".png" || type == ".tiff")
  {
    const double scale = (float)U16_MAX;
    image.convertTo(m, CV_MAKETYPE(CV_16U, 3), scale);
  }
  else
  {
    const double scale = (float)0xff;
    image.convertTo(m, CV_MAKETYPE(CV_8U, 3), scale);
  }
  cv::cvtColor(m, m, cv::COLOR_RGB2BGR);

  std::vector<int32_t> params;
  params.push_back(cv::IMWRITE_JPEG_QUALITY);
  params.push_back(100);
  params.push_back(cv::IMWRITE_PNG_COMPRESSION);
  params.push_back(0);
  return cv::imencode(type, m, buffer, params);
}

bool imEncode(std::vector<uint8_t>& buffer, const std::string& type, const Mat1f& image)
{
  cv::Mat m;
  if (type == ".png" || type == ".tiff")
  {
    const double scale = (float)U16_MAX;
    image.convertTo(m, CV_MAKETYPE(CV_16U, 1), scale);
  }
  else
  {
    const double scale = (float)0xff;
    image.convertTo(m, CV_MAKETYPE(CV_8U, 1), scale);
  }

  std::vector<int32_t> params;
  params.push_back(cv::IMWRITE_JPEG_QUALITY);
  params.push_back(100);
  params.push_back(cv::IMWRITE_PNG_COMPRESSION);
  params.push_back(0);
  return cv::imencode(type, m, buffer, params);
}

bool imEncode(std::vector<uint8_t>& buffer, const std::string& type, const Mat1u& image)
{
  cv::Mat m;
  image.convertTo(m, CV_MAKETYPE(CV_8U, 1), 1.0);

  std::vector<int32_t> params;
  params.push_back(cv::IMWRITE_JPEG_QUALITY);
  params.push_back(100);
  params.push_back(cv::IMWRITE_PNG_COMPRESSION);
  params.push_back(0);
  return cv::imencode(type, m, buffer, params);
}

void imShow(const std::string& windowName, const cv::Mat& a, const int32_t wait_ms)
{
  cv::Mat t;
  if (a.channels() == 3)
  {
    cv::cvtColor(a, t, cv::COLOR_RGB2BGR);
  }
  else
  {
    t = a;
  }

  cv::imshow(windowName, t);
  cv::waitKey(wait_ms);
}

}  // namespace painty
