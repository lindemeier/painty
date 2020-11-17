/**
 * @file CanvasGpuTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-10-15
 *
 */

#include "gtest/gtest.h"
#include "painty/core/KubelkaMunk.hxx"
#include "painty/gpu/GpuMat.hxx"
#include "painty/io/ImageIO.hxx"
#include "painty/renderer/CanvasGpu.hxx"
#include "painty/renderer/SbrRenderThread.hxx"
#include "painty/renderer/TextureBrushGpu.hxx"
#include "prgl/Window.hxx"

const auto windowSize = painty::Size{1024U, 768U};
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
const auto gpuQueue = std::make_shared<painty::GpuTaskQueue>(windowSize);
#pragma clang diagnostic pop

TEST(SbrRenderThreadTest, Construct) {
  painty::SbrRenderThread renderThread(gpuQueue, windowSize);

}

TEST(SbrRenderThreadTest, Smudge) {
  painty::SbrRenderThread renderThread(gpuQueue, windowSize);

  renderThread.enableSmudge(true);
  renderThread.setBrushThicknessScale(1.0);

  constexpr auto radius = 40.0;
  {
    painty::vec3 K = {};
    painty::vec3 S = {};
    painty::ComputeScatteringAndAbsorption(
      painty::vec3{0.01, 0.01, 0.01}, painty::vec3{0.02, 0.02, 0.02}, K, S);
    std::vector<painty::vec2> path = {{50.0, 400.0}, {800.0, 400.0}};
    renderThread.render(path, radius, {K, S}).wait();

    const auto image = renderThread.getLinearRgbImage().get();
    painty::io::imSave("/tmp/Smudge0.jpg", image, true);
  }

  {
    painty::vec3 K = {};
    painty::vec3 S = {};
    painty::ComputeScatteringAndAbsorption(painty::vec3{0.8, 0.8, 0.8},
                                           painty::vec3{0.9, 0.9, 0.9}, K, S);
    std::vector<painty::vec2> path = {{500.0, 50.0}, {500.0, 800.0}};
    renderThread.render(path, radius, {K, S}).wait();

    const auto image = renderThread.getLinearRgbImage().get();
    painty::io::imSave("/tmp/Smudge1.jpg", image, true);
  }
}

TEST(GpuMatTest, Construct) {
  auto f = gpuQueue->add_task([]() {
    painty::GpuMat<painty::vec4f> m(painty::Size{1024U, 768U});

    m.download();
    const auto mat = m.getMat();
    return mat;
  });

  f.get();
}
