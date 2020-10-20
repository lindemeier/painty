/**
 * @file TextureBrushGpu.cxx
 * @author Thomas Lindemeier
 * @brief
 * @date 2020-10-07
 *
 */
#include "painty/renderer/TextureBrushGpu.hxx"

#include "painty/core/Spline.hxx"
#include "painty/io/ImageIO.hxx"
#include "prgl/Projection.hxx"
#include "prgl/Texture2d.hxx"
#include "prgl/VertexArrayObject.hxx"
#include "prgl/VertexBufferObject.hxx"

painty::TextureBrushGpu::TextureBrushGpu()
    : _textureBrushDictionary(),
      _shaderWarp(nullptr),
      _shaderImprint(nullptr),
      _warpedBrushTexture(nullptr),
      _warpedBrushTextureFbo(nullptr) {
  _shaderWarp = prgl::GlslRenderingPipelineProgram::Create();
  _shaderWarp->attachVertexShader(prgl::GlslProgram::ReadShaderFromFile(
    "painty/renderer/shaders/BrushTextureWarp.vert.glsl"));
  _shaderWarp->attachFragmentShader(prgl::GlslProgram::ReadShaderFromFile(
    "painty/renderer/shaders/BrushTextureWarp.frag.glsl"));

  _shaderImprint =
    prgl::GlslComputeShader::Create(prgl::GlslProgram::ReadShaderFromFile(
      "painty/renderer/shaders/PaintTextureFootprintOnCanvas.compute.glsl"));

  _warpedBrushTextureFbo = prgl::FrameBufferObject::Create();
}

void painty::TextureBrushGpu::setRadius(const double radius) {
  constexpr auto Eps = 0.5;
  if (!fuzzyCompare(_radius, radius, Eps)) {
    _radius = radius;
  }
}

void painty::TextureBrushGpu::dip(const std::array<vec3, 2UL>& paint) {
  _paintStored = paint;
}

void painty::TextureBrushGpu::paintStroke(const std::vector<vec2>& verticesArg,
                                          Canvas<vec3>&) {
  if (verticesArg.size() < 2UL) {
    return;
  }
}

void painty::TextureBrushGpu::paintStroke(const std::vector<vec2>& verticesArg,
                                          CanvasGpu& canvas) {
  if (verticesArg.size() < 2UL) {
    return;
  }
  auto vertices = std::vector<vec2>();
  vertices.push_back(verticesArg.front() -
                     (verticesArg[1U] - verticesArg.front()).normalized() *
                       _radius);
  vertices.insert(vertices.end(), verticesArg.cbegin(), verticesArg.cend());
  vertices.push_back(
    verticesArg.back() +
    (verticesArg.back() - verticesArg[verticesArg.size() - 2U]).normalized() *
      _radius);

  // compute bounding rectangle
  auto boundMin = vertices.front();
  auto boundMax = vertices.front();
  for (const auto& xy : vertices) {
    boundMin[0U] = std::min(boundMin[0U], xy[0U]);
    boundMin[1U] = std::min(boundMin[1U], xy[1U]);
    boundMax[0U] = std::max(boundMax[0U], xy[0U]);
    boundMax[1U] = std::max(boundMax[1U], xy[1U]);
  }
  boundMin[0U] = std::max(boundMin[0U] - _radius, 0.0);
  boundMax[0U] = std::min(boundMax[0U] + _radius,
                          static_cast<T>(canvas.getSize().width - 1));
  boundMin[1U] = std::max(boundMin[1U] - _radius, 0.0);
  boundMax[1U] = std::min(boundMax[1U] + _radius,
                          static_cast<T>(canvas.getSize().height - 1));

  generateWarpedTexture(vertices, canvas.getSize());

  {
    prgl::Binder<prgl::GlslComputeShader> shaderBinder(_shaderImprint);

    _shaderImprint->bindImage2D(0U, _warpedBrushTexture,
                                prgl::TextureAccess::ReadOnly);
    _shaderImprint->bindImage2D(1U, canvas.getPaintLayer().getK().getTexture(),
                                prgl::TextureAccess::ReadWrite);
    _shaderImprint->bindImage2D(2U, canvas.getPaintLayer().getS().getTexture(),
                                prgl::TextureAccess::ReadWrite);

    _shaderImprint->set3f("K_brush",
                          static_cast<float>(_paintStored.front()[0U]),
                          static_cast<float>(_paintStored.front()[1U]),
                          static_cast<float>(_paintStored.front()[2U]));
    _shaderImprint->set3f("S_brush",
                          static_cast<float>(_paintStored.back()[0U]),
                          static_cast<float>(_paintStored.back()[1U]),
                          static_cast<float>(_paintStored.back()[2U]));

    _shaderImprint->execute(
      static_cast<int32_t>(boundMin[0U]), static_cast<int32_t>(boundMin[1U]),
      static_cast<int32_t>(boundMax[0U] - boundMin[0U]) + 1,
      static_cast<int32_t>(boundMax[1U] - boundMin[1U]) + 1);
  }
}

void painty::TextureBrushGpu::generateWarpedTexture(
  const std::vector<vec2>& vertices, const Size& size) {
  if ((_warpedBrushTexture == nullptr) ||
      (_warpedBrushTexture->getHeight() != size.height) ||
      (_warpedBrushTexture->getWidth() != size.width)) {
    _warpedBrushTexture = prgl::Texture2d::Create(
      size.width, size.height, prgl::TextureFormatInternal::R32F,
      prgl::TextureFormat::Red, prgl::DataType::Float);
    _warpedBrushTexture->upload(nullptr);
    _warpedBrushTextureFbo->attachTexture(_warpedBrushTexture);
  }
  GLuint clearColor[4] = {0, 0, 0, 0};
  glClearTexImage(_warpedBrushTexture->getId(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                  &clearColor);

  SplineEval<std::vector<vec2>::const_iterator> spineSpline(vertices.cbegin(),
                                                            vertices.cend());

  std::vector<std::array<float, 2U>> vboVertices;
  std::vector<std::array<float, 2U>> vboTexCoords;
  for (auto i = 0U; i < vertices.size(); ++i) {
    T u          = static_cast<T>(i) / static_cast<T>(vertices.size() - 1);
    const auto c = spineSpline.catmullRom(u);

    // spine tangent vector
    auto t = spineSpline.catmullRomDerivativeFirst(u).normalized();

    // compute perpendicular vector to spine
    const vec2 d = {-t[1], t[0]};

    const vec2 l = c - _radius * d;
    const vec2 r = c + _radius * d;

    vboVertices.push_back(
      {static_cast<float>(r[0U]), static_cast<float>(r[1U])});
    vboVertices.push_back(
      {static_cast<float>(l[0U]), static_cast<float>(l[1U])});

    vboTexCoords.push_back({static_cast<float>(u), static_cast<float>(0.0)});
    vboTexCoords.push_back({static_cast<float>(u), static_cast<float>(1.0)});
  }

  auto vboPosition = prgl::VertexBufferObject::Create(
    prgl::VertexBufferObject::Usage::StaticDraw);
  vboPosition->createBuffer(vboVertices);

  auto vboTex = prgl::VertexBufferObject::Create(
    prgl::VertexBufferObject::Usage::StaticDraw);
  vboTex->createBuffer(vboTexCoords);

  auto vao = prgl::VertexArrayObject::Create();
  vao->addVertexBufferObject(0U, vboPosition);
  vao->addVertexBufferObject(1U, vboTex);

  {
    const auto fboBinder =
      prgl::Binder<prgl::FrameBufferObject>(_warpedBrushTextureFbo);
    {
      const auto shaderBinder =
        prgl::Binder<prgl::GlslRenderingPipelineProgram>(_shaderWarp);

      {
        const auto vaoBinder = prgl::Binder<prgl::VertexArrayObject>(vao);

        const auto ortho = prgl::projection::ortho<float>(
          0.0F, static_cast<float>(_warpedBrushTexture->getWidth()),
          static_cast<float>(_warpedBrushTexture->getHeight()), 0.0F, -1.0F,
          1.0F);

        _shaderWarp->setMatrix("projectionMatrix", ortho);
        const auto brushTexture =
          _textureBrushDictionary.lookup(vertices, 2.0 * _radius).texGpu;
        _shaderWarp->bindSampler("brushTexture", 0U, brushTexture);

        vao->render(prgl::DrawMode::TriangleStrip, 0U,
                    static_cast<uint32_t>(vboVertices.size()));
      }
    }
  }
}

void painty::TextureBrushGpu::enableSmudge(bool enable) {
  _smudge = enable;
}
