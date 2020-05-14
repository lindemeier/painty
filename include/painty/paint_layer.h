/**
 * @file mat.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-14
 *
 */
#ifndef PAINTY_PAINT_LAYER_H
#define PAINTY_PAINT_LAYER_H

#include <type_traits>

#include <painty/mat.h>
#include <painty/kubelka_munk.h>

namespace painty
{
/**
 * @brief Stores paint and amount cellwise.
 */
template <class T, size_t N, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
class PaintLayer
{
public:
  PaintLayer(uint32_t rows, uint32_t cols) : _K_buffer(rows, cols), _S_buffer(rows, cols), _V_buffer(rows, cols)
  {
  }

  const Mat<vec<T, N>>& getK_buffer() const
  {
    return _K_buffer;
  }

  const Mat<vec<T, N>>& getS_buffer() const
  {
    return _S_buffer;
  }

  const Mat<T>& getV_buffer() const
  {
    return _V_buffer;
  }

  Mat<vec<T, N>>& getK_buffer()
  {
    return _K_buffer;
  }

  Mat<vec<T, N>>& getS_buffer()
  {
    return _S_buffer;
  }

  Mat<T>& getV_buffer()
  {
    return _V_buffer;
  }

  uint32_t getCols() const
  {
    return _K_buffer.getCols();
  }

  uint32_t getRows() const
  {
    return _K_buffer.getRows();
  }

  /**
   * @brief Set all values to zero.
   *
   */
  void clear()
  {
    auto& K = _K_buffer.getData();
    auto& S = _S_buffer.getData();
    auto& V = _V_buffer.getData();
    for (size_t i = 0; i < K.size(); i++)
    {
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
  void composeOnto(Mat<vec<T, N>>& R0) const
  {
    if ((R0.getRows() != _K_buffer.getRows()) || (R0.getCols() != _K_buffer.getCols()))
    {
      R0 = Mat<vec<T, N>>(_K_buffer.getRows(), _K_buffer.getCols());
      for (auto& v : R0.getData())
      {
        v.fill(1.0);
      }
    }

    auto& K = _K_buffer.getData();
    auto& S = _S_buffer.getData();
    auto& V = _V_buffer.getData();
    auto& R0_data = R0.getData();
    for (size_t i = 0UL; i < K.size(); i++)
    {
      R0_data[i] = ComputeReflectance(K[i], S[i], R0_data[i], V[i]);
    }
  }

  /**
   * @brief Deep copy this buffer to another.
   *
   * @param other
   */
  void copyTo(PaintLayer& other) const
  {
    if ((other.getRows() != _K_buffer.getRows()) || (other.getCols() != _K_buffer.getCols()))
    {
      other = PaintLayer<T, N>(getRows(), getCols());
    }
    std::copy(_K_buffer.getData().cbegin(), _K_buffer.getData().cend(), other._K_buffer.getData().begin());
    std::copy(_S_buffer.getData().cbegin(), _S_buffer.getData().cend(), other._S_buffer.getData().begin());
    std::copy(_V_buffer.getData().cbegin(), _V_buffer.getData().cend(), other._V_buffer.getData().begin());
  }

private:
  /**
   * @brief Absorption
   *
   */
  Mat<vec<T, N>> _K_buffer;
  /**
   * @brief Scattering
   *
   */
  Mat<vec<T, N>> _S_buffer;
  /**
   * @brief Amount of paint.
   *
   */
  Mat<T> _V_buffer;
};
}  // namespace painty

#endif  // PAINTY_PAINT_LAYER_H
