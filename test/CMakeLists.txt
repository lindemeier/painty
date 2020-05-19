cmake_minimum_required(VERSION 3.16.0)

# get google test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)
FetchContent_MakeAvailable(googletest)

project(painty_test)

add_executable(${PROJECT_NAME}
  brush_stroke_sample_test.cpp
  canvas_test.cpp
  kubelka_munk_test.cpp
  image_io_test.cpp
  main.cpp
  mat_test.cpp
  math_test.cpp
  paint_layer_test.cpp
  spline_test.cpp
  texture_brush_test.cpp
  vec_test.cpp
)

target_link_libraries(${PROJECT_NAME}
  gtest
  painty
)

add_dependencies(${PROJECT_NAME}
  painty
)


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Weverything -Wno-c++98-compat -Wno-padded -Wno-documentation -Werror -Wno-global-constructors)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
endif()
