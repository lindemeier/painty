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

TEST(GpuMatTest, Construct) {
  auto f = gpuQueue->add_task([]() {
    painty::GpuMat<painty::vec4f> m(painty::Size{1024U, 768U});

    m.download();
    const auto mat = m.getMat();
    return mat;
  });

  f.get();
}
