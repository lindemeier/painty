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

void painty::PaintLayerGpu::composeOnto(GpuMat<vec3f>& R0) const {
  const auto shader =
    prgl::GlslComputeShader::Create(prgl::GlslProgram::ReadShaderFromFile(
      "painty/renderer/shaders/ComposeLayerOnSubstrate.compute.glsl"));

}
