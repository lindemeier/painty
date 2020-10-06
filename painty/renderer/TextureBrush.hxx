/**
 * @file TextureBrush.h
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-05-19
 *
 */
#pragma once

#include "painty/core/Spline.hxx"
#include "painty/renderer/BrushBase.hxx"
#include "painty/renderer/BrushStrokeSample.hxx"
#include "painty/renderer/Canvas.hxx"
#include "painty/renderer/Smudge.hxx"
#include "painty/renderer/TextureBrushDictionary.hxx"
#include "prgl/FrameBufferObject.hxx"
#include "prgl/GlslRenderingPipelineProgram.hxx"
#include "prgl/Projection.hxx"
#include "prgl/Texture2d.hxx"
#include "prgl/VertexArrayObject.hxx"
#include "prgl/VertexBufferObject.hxx"

namespace painty {
template <class vector_type>
class TextureBrush final : public BrushBase<vector_type> {
  using T                 = typename BrushBase<vector_type>::T;
  static constexpr auto N = BrushBase<vector_type>::N;

 public:
  TextureBrush(const std::string& sampleDir)
      : _brushStrokeSample(sampleDir),
        _smudge(static_cast<int32_t>(2.0 * _radius)),
        _textureBrushDictionary() {
    for (auto& c : _paintStored) {
      c.fill(static_cast<T>(0.1));
    }
  }

  void setRadius(const double radius) override {
    constexpr auto Eps = 0.5;
    if (!fuzzyCompare(_radius, radius, Eps)) {
      _radius = radius;
      if (_useSmudge) {
        _smudge = Smudge<vector_type>(static_cast<int32_t>(2.0 * radius));
      }
    }
  }

  /**
   * @brief Dip the brush in paint paint.
   *
   * @param paint
   */
  void dip(const std::array<vector_type, 2UL>& paint) override {
    _paintStored = paint;
  }

  void paintStroke(const std::vector<vec2>& verticesArg,
                   Canvas<vector_type>& canvas) override {
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
    boundMax[0U] =
      std::min(boundMax[0U] + _radius,
               static_cast<T>(canvas.getPaintLayer().getCols() - 1));
    boundMin[1U] = std::max(boundMin[1U] - _radius, 0.0);
    boundMax[1U] =
      std::min(boundMax[1U] + _radius,
               static_cast<T>(canvas.getPaintLayer().getRows() - 1));

    auto length = 0.0;
    for (auto i = 1UL; i < vertices.size(); ++i) {
      length += (vertices[i] - vertices[i - 1]).norm();
    }

    // spine
    SplineEval<std::vector<vec2>::const_iterator> spineSpline(vertices.cbegin(),
                                                              vertices.cend());

    // construct frames around the vertices
    std::vector<vec2> upCanvasCoordinates;
    upCanvasCoordinates.reserve(vertices.size());
    std::vector<vec2> downCanvasCoordinates;
    downCanvasCoordinates.reserve(vertices.size());

    // frames around the texture
    std::vector<vec2> upUv;
    upUv.reserve(vertices.size());
    std::vector<vec2> downUv;
    downUv.reserve(vertices.size());

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

      upCanvasCoordinates.push_back(l);
      downCanvasCoordinates.push_back(r);

      vboVertices.push_back(
        {static_cast<float>(r[0U]), static_cast<float>(r[1U])});
      vboVertices.push_back(
        {static_cast<float>(l[0U]), static_cast<float>(l[1U])});

      vboTexCoords.push_back({static_cast<float>(u), static_cast<float>(0.0)});
      vboTexCoords.push_back({static_cast<float>(u), static_cast<float>(1.0)});

      // constexpr auto uvM = 1.0;
      upUv.push_back({u, 0.0});
      downUv.push_back({u, 1.0});
    }
    upCanvasCoordinates.insert(upCanvasCoordinates.begin(),
                               downCanvasCoordinates.rbegin(),
                               downCanvasCoordinates.rend());

    upUv.insert(upUv.begin(), downUv.rbegin(), downUv.rend());

    const auto now = std::chrono::system_clock::now();

    Mat<T> thicknessMap(static_cast<int32_t>(boundMax[1] - boundMin[1] + 1),
                        static_cast<int32_t>(boundMax[0] - boundMin[0] + 1));
    for (auto& p : thicknessMap) {
      p = static_cast<T>(0.0);
    }
    auto heightTexture = prgl::Texture2d::Create(
      canvas.getPaintLayer().getCols(), canvas.getPaintLayer().getRows(),
      prgl::TextureFormatInternal::R8, prgl::TextureFormat::Red,
      prgl::DataType::Byte);
    {
      std::vector<uint8_t> empty(
        heightTexture->getWidth() * heightTexture->getHeight(),
        static_cast<uint8_t>(0U));

      heightTexture->upload(empty.data());
    }
    auto heightFbo = prgl::FrameBufferObject::Create();
    heightFbo->attachTexture(heightTexture);

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
    glsl->attachVertexShader(R"(
      #version 330 core

      layout(location = 0) in vec2 vertexPosition;
      layout(location = 1) in vec2 vertexTexCoord;

      uniform mat4 projectionMatrix;

      out vec2 texCoord;

      void main()
      {
        texCoord = vertexTexCoord;

        gl_Position = projectionMatrix * vec4(vertexPosition.xy, 0.0, 1.0);
      }
    )");
    glsl->attachFragmentShader(R"(
      #version 420 core

      layout(binding = 0) uniform sampler2D brushTexture;

      in vec2 texCoord;
      out vec3 color;

      void main()
      {
        color = texture(brushTexture, texCoord).rgb;
        // color = vec3(1.0,1.0, 1.0);
      }
    )");
    const auto brushTexture = _brushStrokeSample.getThicknessMap();
    auto brushTextureGl     = prgl::Texture2d::Create(
      brushTexture.cols, brushTexture.rows, prgl::TextureFormatInternal::R8,
      prgl::TextureFormat::Red, prgl::DataType::Byte);
    const auto fboBinder = prgl::Binder<prgl::FrameBufferObject>(heightFbo);
    {
      const auto shaderBinder =
        prgl::Binder<prgl::GlslRenderingPipelineProgram>(glsl);

      {
        const auto vaoBinder = prgl::Binder<prgl::VertexArrayObject>(vao);
        const auto ortho     = prgl::projection::ortho<float>(
          0.0F, static_cast<float>(heightTexture->getWidth()),
          static_cast<float>(heightTexture->getHeight()), 0.0F, -1.0F, 1.0F);
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
    std::vector<float> heightTextureData;
    heightTexture->download(heightTextureData);

    Mat1d texDataMat(static_cast<int32_t>(heightTexture->getHeight()),
                     static_cast<int32_t>(heightTexture->getWidth()));
    for (auto i = 0U; i < heightTextureData.size(); i++) {
      texDataMat(static_cast<int32_t>(i)) =
        static_cast<double>(heightTextureData[i]);
    }
    cv::flip(texDataMat, texDataMat, 0);
    io::imSave("/tmp/heightTextureData.jpg", texDataMat, false);

    std::vector<vec<int32_t, 2U>> pixels;
    for (auto x = static_cast<int32_t>(boundMin[0U]);
         x <= static_cast<int32_t>(boundMax[0U]); x++) {
      for (auto y = static_cast<int32_t>(boundMin[1U]);
           y <= static_cast<int32_t>(boundMax[1U]); y++) {
        if ((x < 0) || (x >= canvas.getPaintLayer().getCols()) || (y < 0) ||
            (y >= canvas.getPaintLayer().getRows())) {
          continue;
        }

        auto texPos = generalizedBarycentricCoordinatesInterpolate(
          upCanvasCoordinates, {x, y}, upUv);
        if ((texPos[0U] < 0.0) || (texPos[0U] > 1.0) || (texPos[1U] < 0.0) ||
            (texPos[1U] > 1.0)) {
          continue;
        }
        texPos[0U] *= _brushStrokeSample.getThicknessMap().cols;
        texPos[1U] *= _brushStrokeSample.getThicknessMap().rows;
        const auto Vtex =
          BrushBase<vector_type>::getThicknessScale() *
          Interpolate(_brushStrokeSample.getThicknessMap(), texPos);
        if (Vtex > 0.0) {
          const auto s = x - static_cast<int32_t>(boundMin[0U]);
          const auto t = y - static_cast<int32_t>(boundMin[1U]);
          if ((s >= 0) && (t >= 0) && (s < thicknessMap.cols) &&
              (t < thicknessMap.rows)) {
            canvas.checkDry(x, y, now);
            thicknessMap(t, s) = Vtex;
            pixels.emplace_back(x, y);
          }
        }
      }
    }

    if (_useSmudge) {
      _smudge.smudge(canvas, boundMin, spineSpline, length, thicknessMap);
    }

    for (const auto& p : pixels) {
      auto& vBuffer = canvas.getPaintLayer().getV_buffer();
      const auto x  = p[0U];
      const auto y  = p[1U];
      if ((x < 0) || (y < 0) || (x >= vBuffer.cols) || (y >= vBuffer.rows)) {
        continue;
      }
      const auto Vtex = thicknessMap(
        clamp(0, y - static_cast<int32_t>(boundMin[1U]), thicknessMap.rows - 1),
        clamp(0, x - static_cast<int32_t>(boundMin[0U]),
              thicknessMap.cols - 1));
      const auto Vcan = vBuffer(y, x);

      const auto Vsum = Vcan + Vtex;
      if (Vsum > static_cast<T>(0.0)) {
        const T sc = static_cast<T>(1.0) / Vsum;

        auto& K = canvas.getPaintLayer().getK_buffer()(y, x);
        auto& S = canvas.getPaintLayer().getS_buffer()(y, x);
        auto& V = vBuffer(y, x);

        K = (Vcan * K + Vtex * _paintStored[0U]) * sc;
        S = (Vcan * S + Vtex * _paintStored[1U]) * sc;
        V = std::max(Vtex, Vcan);
      }
    }
  }

  void enableSmudge(const bool enable) {
    _useSmudge = enable;
  }

 private:
  /**
   * @brief Brush stroke texture sample that can be warped along a trajectory or list of vertices.
   *
   */
  BrushStrokeSample _brushStrokeSample;

  /**
   * @brief The current paint the brush stores.
   *
   */
  std::array<vector_type, 2UL> _paintStored;

  /**
   * @brief
   *
   */
  double _radius = 0.0;

  /**
   * @brief
   *
   */
  Smudge<vector_type> _smudge;

  bool _useSmudge = true;

  /**
   * @brief Retrieve brush textures according to brush radii and path length.
   *
   */
  TextureBrushDictionary _textureBrushDictionary;
};
}  // namespace painty
