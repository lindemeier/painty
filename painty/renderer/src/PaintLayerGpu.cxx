/**
 * @file PaintLayerGpu.cxx
 * @author thomas lindemeier
 * @brief
 * @date 2020-10-08
 *
 */
#include "painty/renderer/PaintLayerGpu.hxx"

#include "prgl/GlslComputeShader.hxx"

painty::PaintLayerGpu::PaintLayerGpu(const Size& size)
    : _size(size),
      _K(_size),
      _S(_size),
      _V(_size) {
  clear();
}

void painty::PaintLayerGpu::clear() {
  _K.clear();
  _S.clear();
  _V.clear();
}

void painty::PaintLayerGpu::composeOnto(GpuMat<vec4f>& R0) const {
  const auto shader =
    prgl::GlslComputeShader::Create(prgl::GlslProgram::ReadShaderFromFile(
      "painty/renderer/shaders/ComposeLayerOnSubstrate.compute.glsl"));

  shader->bind(true);

  shader->bindImage2D(0U, R0.getTexture(), prgl::TextureAccess::ReadWrite);
  shader->bindImage2D(1U, _K.getTexture(), prgl::TextureAccess::ReadOnly);
  shader->bindImage2D(2U, _S.getTexture(), prgl::TextureAccess::ReadOnly);
  shader->bindImage2D(3U, _V.getTexture(), prgl::TextureAccess::ReadOnly);

  shader->execute(0, 0, static_cast<int32_t>(_size.width),
                  static_cast<int32_t>(_size.height));

  shader->bind(false);
}
