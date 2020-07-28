/**
 * @file paint_layer.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-14
 *
 */
#pragma once

#include <painty/core/kubelka_munk.h>
#include <painty/image/mat.h>

#include <type_traits>

namespace painty {
/**
 * @brief Stores paint and amount cellwise.
 */
template <class vector_type>
class PaintLayer final {
  using T                 = typename DataType<vector_type>::channel_type;
  static constexpr auto N = DataType<vector_type>::dim;

 public:
  PaintLayer(uint32_t rows, uint32_t cols)
      : _K_buffer(rows, cols),
        _S_buffer(rows, cols),
        _V_buffer(rows, cols) {}

  const Mat<vector_type>& getK_buffer() const {
    return _K_buffer;
  }

  const Mat<vector_type>& getS_buffer() const {
    return _S_buffer;
  }

  const Mat<T>& getV_buffer() const {
    return _V_buffer;
  }

  Mat<vector_type>& getK_buffer() {
    return _K_buffer;
  }

  Mat<vector_type>& getS_buffer() {
    return _S_buffer;
  }

  Mat<T>& getV_buffer() {
    return _V_buffer;
  }

  uint32_t getCols() const {
    return _K_buffer.getCols();
  }

  uint32_t getRows() const {
    return _K_buffer.getRows();
  }

  /**
   * @brief Set all values to zero.
   *
   */
  void clear() {
    auto& K = _K_buffer.getData();
    auto& S = _S_buffer.getData();
    auto& V = _V_buffer.getData();
    for (size_t i = 0; i < K.size(); i++) {
      K[i].fill(static_cast<T>(0.0));
      S[i].fill(static_cast<T>(0.0));
      V[i] = static_cast<T>(0.0);
    }
  }

  /**
   * @brief Compose this layer onto a substrate (reflectance). The layer is assumed to be dry.
   *
   * @param R0 the substrate
   */
  void composeOnto(Mat<vector_type>& R0) const {
    if ((R0.getRows() != _K_buffer.getRows()) ||
        (R0.getCols() != _K_buffer.getCols())) {
      R0 = Mat<vector_type>(_K_buffer.getRows(), _K_buffer.getCols());
      for (auto& v : R0.getData()) {
        v.fill(1.0);
      }
    }

    auto& K       = _K_buffer.getData();
    auto& S       = _S_buffer.getData();
    auto& V       = _V_buffer.getData();
    auto& R0_data = R0.getData();
    for (size_t i = 0UL; i < K.size(); i++) {
      R0_data[i] = ComputeReflectance(K[i], S[i], R0_data[i], V[i]);
    }
  }

  /**
   * @brief Deep copy this buffer to another.
   *
   * @param other
   */
  void copyTo(PaintLayer& other) const {
    if ((other.getRows() != _K_buffer.getRows()) ||
        (other.getCols() != _K_buffer.getCols())) {
      other = PaintLayer<vector_type>(getRows(), getCols());
    }
    std::copy(_K_buffer.getData().cbegin(), _K_buffer.getData().cend(),
              other._K_buffer.getData().begin());
    std::copy(_S_buffer.getData().cbegin(), _S_buffer.getData().cend(),
              other._S_buffer.getData().begin());
    std::copy(_V_buffer.getData().cbegin(), _V_buffer.getData().cend(),
              other._V_buffer.getData().begin());
  }

  /**
   * @brief Update a cell of the canvas.
   *
   * @param i the row index
   * @param j the col index
   * @param k Scattering
   * @param s Absorption
   * @param v Volume
   */
  void set(uint32_t i, uint32_t j, const vector_type& k, const vector_type& s,
           const T v) {
    _K_buffer(i, j) = k;
    _S_buffer(i, j) = s;
    _V_buffer(i, j) = v;
  }

 private:
  /**
   * @brief Absorption
   *
   */
  Mat<vector_type> _K_buffer;
  /**
   * @brief Scattering
   *
   */
  Mat<vector_type> _S_buffer;
  /**
   * @brief Amount of paint.
   *
   */
  Mat<T> _V_buffer;
};
}  // namespace painty
