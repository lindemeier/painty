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
   * @brief Deep copy.
   *
   * @return Mat<T>
   */
  Mat<T> clone()
  {
    Mat<T> c(_rows, _cols);
    std::copy(_data_ptr->cbegin(), _data_ptr->cend(), c._data_ptr->begin());
    return c;
  }

private:
  inline size_t one_d(uint32_t i, uint32_t j) const
  {
    return i * _cols + j;
  }

  uint32_t _rows = 0U;
  uint32_t _cols = 0U;

  std::shared_ptr<std::vector<T>> _data_ptr;
};

}  // namespace painty

#endif  // PAINTY_MAT_H
