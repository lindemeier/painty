#ifndef PAINTY_TYPES_H
#define PAINTY_TYPES_H

#include <array>

#include <opencv2/imgproc/imgproc.hpp>

namespace cv
{  // opencv access data traits
template <class T, size_t N>
class DataType<std::array<T, N>>
{
public:
  typedef std::array<T, N> value_type;
  typedef std::array<T, N> work_type;
  typedef T channel_type;
  typedef value_type vec_type;
  enum
  {
    generic_type = 0,
    depth = DataDepth<channel_type>::value,
    channels = N,
    fmt = ((channels - 1) << 8) + DataDepth<channel_type>::fmt,
    type = CV_MAKETYPE(depth, channels)
  };
};

}  // namespace cv

namespace painty
{
// TODO eigen or std array
using Mat3f = cv::Mat_<std::array<float, 3>>;
using Mat4f = cv::Mat_<std::array<float, 4>>;
}  // namespace painty

#endif  // PAINTY_TYPES_H
