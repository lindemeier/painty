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
      _composeShader(
        prgl::GlslComputeShader::Create(prgl::GlslProgram::ReadShaderFromFile(
          "painty/renderer/shaders/ComposeLayerOnSubstrate.compute.glsl"))) {
  clear();
}

void painty::PaintLayerGpu::clear() {
  _K.clear();
  _S.clear();
}

void painty::PaintLayerGpu::composeOnto(GpuMat<vec4f>& R0) const {
  _composeShader->bind(true);

  _composeShader->bindImage2D(0U, R0.getTexture(),
                              prgl::TextureAccess::ReadWrite);
  _composeShader->bindImage2D(1U, _K.getTexture(),
                              prgl::TextureAccess::ReadOnly);
  _composeShader->bindImage2D(2U, _S.getTexture(),
                              prgl::TextureAccess::ReadOnly);

  _composeShader->execute(0, 0, static_cast<int32_t>(_size.width),
                          static_cast<int32_t>(_size.height));

  _composeShader->bind(false);
}

auto painty::PaintLayerGpu::getK() -> GpuMat<vec4f>& {
  return _K;
}

auto painty::PaintLayerGpu::getS() -> GpuMat<vec4f>& {
  return _S;
}
