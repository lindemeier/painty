/**
 * @file image_io_test.cpp
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-04-22
 *
 */

#include <gtest/gtest.h>

#include <painty/painty.h>

TEST(ImageIoTest, Read)
{
  painty::Mat<painty::vec3> srgb;

  painty::io::imRead("/home/tsl/Pictures/140467.png", srgb);

  std::cout << srgb.getCols() << " " << srgb.getRows() << std::endl;

  const auto& data = srgb.getData();
  for (uint32_t i = 0U; i < data.size(); i += 1000)
  {
    std::cout << data[i] << std::endl;
  }
}
