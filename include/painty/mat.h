/**
 * @file mat.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */
#ifndef PAINTY_MAT_H
#define PAINTY_MAT_H

#include <memory>
#include <vector>

#include <painty/vec.h>

namespace painty
{
template <class T>
class Mat
{
public:
  explicit Mat() : _rows(0U), _cols(0U), _data_ptr(nullptr)
  {
  }

  explicit Mat(uint32_t rows, uint32_t cols)
    : _rows(rows), _cols(cols), _data_ptr(std::make_shared<std::vector<T>>(rows * cols))
  {
  }

  uint32_t getCols() const
  {
    return _cols;
  }

  uint32_t getRows() const
  {
    return _rows;
  }

  bool isEmpty() const
  {
    return !_data_ptr;
  }

  const T& operator()(uint32_t i, uint32_t j) const
  {
    return (*_data_ptr)[one_d(i, j)];
  }

  T& operator()(uint32_t i, uint32_t j)
  {
    return (*_data_ptr)[one_d(i, j)];
  }

  /**
   * @brief Access data bilinearly interpolated.
   *
   * @param position
   *
   * @return T bilinearly interpolated value at position.
   */
  T operator()(const vec2& position) const
  {
    const auto x = static_cast<int32_t>(std::floor(position[0]));
    const auto y = static_cast<int32_t>(std::floor(position[1]));

    const auto x0 = BorderHandle(x, _cols);
    const auto x1 = BorderHandle(x + 1, _cols);
    const auto y0 = BorderHandle(y, _rows);
    const auto y1 = BorderHandle(y + 1, _rows);

    const auto a = position[0] - static_cast<double>(x);
    const auto c = position[1] - static_cast<double>(y);
    if constexpr (DataType<T>::dim == 1U)
    {
      return static_cast<T>((static_cast<double>(this->operator()(y0, x0)) * (1.0 - a) +
                             static_cast<double>(this->operator()(y0, x1)) * a) *
                                (1.0 - c) +
                            (static_cast<double>(this->operator()(y1, x0)) * (1.0 - a) +
                             static_cast<double>(this->operator()(y1, x1)) * a) *
                                c);
    }
    else
    {
      T r;
      for (auto i = 0U; i < DataType<T>::rows; i++)
      {
        for (auto j = 0U; j < DataType<T>::cols; j++)
        {
          r(i, j) = static_cast<typename DataType<T>::channel_type>(
              (static_cast<double>(this->operator()(y0, x0)(i, j)) * (1.0 - a) +
               static_cast<double>(this->operator()(y0, x1)(i, j)) * a) *
                  (1.0 - c) +
              (static_cast<double>(this->operator()(y1, x0)(i, j)) * (1.0 - a) +
               static_cast<double>(this->operator()(y1, x1)(i, j)) * a) *
                  c);
        }
      }
      return r;
    }
  }

  const std::vector<T>& getData() const
  {
    return *_data_ptr;
  }

  std::vector<T>& getData()
  {
    return *_data_ptr;
  }

  /**
   * @brief Deep copy.
   *
   * @return Mat<T>
   */
  Mat<T> clone() const
  {
    Mat<T> c(_rows, _cols);
    std::copy(_data_ptr->cbegin(), _data_ptr->cend(), c._data_ptr->begin());
    return c;
  }

  /**
   * @brief up- or downsample an image using bilinear interpolation.
   *
   * @param rows number of rows of the resized image
   * @param cols number of cols of the resized image
   *
   * @return Mat<T>
   */
  Mat<T> scaled(const uint32_t rows, const uint32_t cols) const
  {
    Mat<T> s(rows, cols);
    for (auto i = 0U; i < rows; i++)
    {
      for (auto j = 0U; j < cols; j++)
      {
        vec2 p = { (static_cast<double>(j) / static_cast<double>(cols - 1U)) * static_cast<double>(_cols - 1U),
                   (static_cast<double>(i) / static_cast<double>(rows - 1U)) * static_cast<double>(_rows - 1U) };

        s(i, j) = (*this)(p);
      }
    }
    return s;
  }

private:
  inline size_t one_d(uint32_t i, uint32_t j) const
  {
    return i * _cols + j;
  }

  static uint32_t BorderHandle(int32_t pos, uint32_t axisLength)
  {
    if (axisLength == 1U)
    {
      return 0U;
    }
    do
    {
      if (pos < 0)
      {
        pos = -pos - 1;
      }
      else
      {
        pos = static_cast<int32_t>(axisLength) - 1 - (pos - static_cast<int32_t>(axisLength));
      }
    } while (static_cast<uint32_t>(pos) >= axisLength);
    return static_cast<uint32_t>(pos);
  }

  uint32_t _rows = 0U;
  uint32_t _cols = 0U;

  std::shared_ptr<std::vector<T>> _data_ptr = nullptr;
};

}  // namespace painty

#endif  // PAINTY_MAT_H
