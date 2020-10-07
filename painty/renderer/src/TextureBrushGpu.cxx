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
#include "prgl/FrameBufferObject.hxx"
#include "prgl/GlslRenderingPipelineProgram.hxx"
#include "prgl/Projection.hxx"
#include "prgl/Texture2d.hxx"
#include "prgl/VertexArrayObject.hxx"
#include "prgl/VertexBufferObject.hxx"

painty::TextureBrushGpu::TextureBrushGpu(
  const std::shared_ptr<prgl::Window>& window)
    : _glWindow(window),
      _brushStrokeSample(),
      _textureBrushDictionary() {}

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
                                          Canvas<vec3>& canvas) {
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

  _brushStrokeSample.generateFromTexture(
    _textureBrushDictionary.lookup(vertices, 2.0 * _radius));

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

  auto warpedBrushTexture = prgl::Texture2d::Create(
    canvas.getPaintLayer().getCols(), canvas.getPaintLayer().getRows(),
    prgl::TextureFormatInternal::R8, prgl::TextureFormat::Red,
    prgl::DataType::Byte);
  {
    std::vector<uint8_t> empty(
      warpedBrushTexture->getWidth() * warpedBrushTexture->getHeight(),
      static_cast<uint8_t>(0U));

    warpedBrushTexture->upload(empty.data());
  }
  auto heightFbo = prgl::FrameBufferObject::Create();
  heightFbo->attachTexture(warpedBrushTexture);

  auto vboPosition = prgl::VertexBufferObject::Create(
    prgl::VertexBufferObject::Usage::StaticDraw);
  vboPosition->createBuffer(vboVertices);

  auto vboTex = prgl::VertexBufferObject::Create(
    prgl::VertexBufferObject::Usage::StaticDraw);
  vboTex->createBuffer(vboTexCoords);

  auto vao = prgl::VertexArrayObject::Create();
  vao->addVertexBufferObject(0U, vboPosition);
  vao->addVertexBufferObject(1U, vboTex);

  auto glsl = prgl::GlslRenderingPipelineProgram::Create();
  glsl->attachVertexShader(prgl::GlslProgram::ReadShaderFromFile(
    "painty/renderer/shaders/BrushTextureWarp.vert.glsl"));
  glsl->attachFragmentShader(prgl::GlslProgram::ReadShaderFromFile(
    "painty/renderer/shaders/BrushTextureWarp.frag.glsl"));

  const auto brushTexture = _brushStrokeSample.getThicknessMap();
  auto brushTextureGl     = prgl::Texture2d::Create(
    brushTexture.cols, brushTexture.rows, prgl::TextureFormatInternal::R8,
    prgl::TextureFormat::Red, prgl::DataType::Byte);
  {
    const auto fboBinder = prgl::Binder<prgl::FrameBufferObject>(heightFbo);
    {
      const auto shaderBinder =
        prgl::Binder<prgl::GlslRenderingPipelineProgram>(glsl);

      {
        const auto vaoBinder = prgl::Binder<prgl::VertexArrayObject>(vao);

        const auto ortho = prgl::projection::ortho<float>(
          0.0F, static_cast<float>(warpedBrushTexture->getWidth()),
          static_cast<float>(warpedBrushTexture->getHeight()), 0.0F, -1.0F,
          1.0F);

        glsl->setMatrix("projectionMatrix", ortho);

        // gl mat

        Mat1d copy;
        cv::flip(brushTexture, copy, 0);
        cv::normalize(copy, copy, cv::NORM_MINMAX, 1.0);
        Mat1u copy2;
        copy.convertTo(copy2, CV_8UC1, 255.0);
        brushTextureGl->upload(copy2.data);
        // gl mat

        glsl->bindSampler("brushTexture", 0U, brushTextureGl);

        vao->render(prgl::DrawMode::TriangleStrip, 0U,
                    static_cast<uint32_t>(vboVertices.size()));
        // vao->render(prgl::DrawMode::LineStrip, 0U,
        //             static_cast<uint32_t>(vboVertices.size()));
      }
    }
  }
  std::vector<float> warpedBrushTextureData;
  warpedBrushTexture->download(warpedBrushTextureData);

  Mat1d texDataMat(static_cast<int32_t>(warpedBrushTexture->getHeight()),
                   static_cast<int32_t>(warpedBrushTexture->getWidth()));
  for (auto i = 0U; i < warpedBrushTextureData.size(); i++) {
    texDataMat(static_cast<int32_t>(i)) =
      static_cast<double>(warpedBrushTextureData[i]);
  }
  cv::flip(texDataMat, texDataMat, 0);
  io::imSave("/tmp/warpedBrushTextureDataGlsl.jpg", texDataMat, false);
}
