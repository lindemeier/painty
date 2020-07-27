/**
 * @file types.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */
#ifndef PAINTY_TYPES_H
#define PAINTY_TYPES_H

#include <stdint.h>

#include <string>

namespace painty {
template <typename T>
class DataType {
 public:
  using channel_type             = T;
  static constexpr uint32_t rows = 1;
  static constexpr uint32_t cols = 1;
  static constexpr uint32_t dim  = 1;

  static std::string getName() {
    return typeid(T).name();
  }
};

template <>
class DataType<float> {
 public:
  using channel_type             = float;
  static constexpr uint32_t rows = 1;
  static constexpr uint32_t cols = 1;
  static constexpr uint32_t dim  = 1;

  static std::string getName() {
    return "float";
  }
};

template <>
class DataType<double> {
 public:
  using channel_type             = double;
  static constexpr uint32_t rows = 1;
  static constexpr uint32_t cols = 1;
  static constexpr uint32_t dim  = 1;

  static std::string getName() {
    return "double";
  }
};
}  // namespace painty

#endif  // PAINTY_TYPES_H
