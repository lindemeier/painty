/**
 * @file ConvolutionTest.cxx
 * @author thomas lindemeier
 *
 * @brief
 *
 * @date 2020-08-26
 *
 */
#include <fstream>

#include "gtest/gtest.h"
#include "painty/mixer/Palette.hxx"
#include "painty/mixer/Serialization.hxx"

TEST(SerializationTest, SaveLoadTest) {
  /*
   * Cassidy J. Curtis, Sean E. Anderson, Joshua E. Seims, Kurt W.
   * Fleischer, and David H. Salesin. 1997. Computer-generated
   * watercolor. In Proceedings of the 24th annual conference on
   * Computer graphics and interactive techniques (SIGGRAPH '97). ACM
   * Press/Addison-Wesley Publishing Co., New York, NY, USA, 421-430.
   * DOI=http://dx.doi.org/10.1145/258734.258896
   * */
  //    http://grail.cs.washington.edu/projects/watercolor/paper_small.pdf

  painty::Palette palette = {};
  palette.push_back({{0.0, 0.5, 1.0}, {0.2, 0.3, 0.4}});

  //    “Quinacridone Rose”
  palette.push_back({{0.22, 1.47, 0.57}, {0.05, 0.003, 0.03}});

  //    “Indian Red”
  palette.push_back({{0.46, 1.07, 1.50}, {1.28, 0.38, 0.21}});

  //    “Cadmium Yellow”
  palette.push_back({{0.10, 0.36, 3.45}, {0.97, 0.65, 0.007}});

  //    “Hookers Green”
  palette.push_back({{1.62, 0.61, 1.64}, {0.01, 0.012, 0.003}});

  //    “Cerulean Blue”
  palette.push_back({{1.52, 0.32, 0.25}, {0.06, 0.26, 0.40}});

  //    “Burnt Umber”
  palette.push_back({{0.74, 1.54, 2.10}, {0.09, 0.09, 0.004}});

  //    “Cadmium Red”
  palette.push_back({{0.14, 1.08, 1.68}, {0.77, 0.015, 0.018}});

  //    “Brilliant Orange”
  palette.push_back({{0.13, 0.81, 3.45}, {0.005, 0.009, 0.007}});

  //    “Hansa Yellow”
  palette.push_back({{0.06, 0.21, 1.78}, {0.50, 0.88, 0.009}});

  //    “Phthalo Green”
  palette.push_back({{1.55, 0.47, 0.63}, {0.01, 0.05, 0.035}});

  //    “French Ultramarine”
  palette.push_back({{0.86, 0.86, 0.06}, {0.005, 0.005, 0.09}});

  //    “Interference Lilac”
  palette.push_back({{0.08, 0.11, 0.07}, {1.25, 0.42, 1.43}});

  auto fout = std::ofstream();
  fout.open("/tmp/test_palette.json");
  painty::SavePalette(fout, palette);
  fout.close();

  auto fin = std::ifstream();
  fin.open("/tmp/test_palette.json");
  painty::Palette palette_loaded = {};
  painty::LoadPalette(fin, palette_loaded);
  fin.close();
  EXPECT_EQ(palette_loaded.size(), palette.size());
  for (auto i = 0UL; i < palette.size(); i++) {
    constexpr auto Eps = 10e-9;
    EXPECT_NEAR(palette[i].K[0U], palette_loaded[i].K[0U], Eps);
    EXPECT_NEAR(palette[i].K[1U], palette_loaded[i].K[1U], Eps);
    EXPECT_NEAR(palette[i].K[2U], palette_loaded[i].K[2U], Eps);
    EXPECT_NEAR(palette[i].S[0U], palette_loaded[i].S[0U], Eps);
    EXPECT_NEAR(palette[i].S[1U], palette_loaded[i].S[1U], Eps);
    EXPECT_NEAR(palette[i].S[2U], palette_loaded[i].S[2U], Eps);
  }
}
