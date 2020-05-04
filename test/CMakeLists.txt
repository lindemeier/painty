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
  main.cpp
  mat_test.cpp
  math_test.cpp
  image_io_test.cpp
  vec_test.cpp
)

target_link_libraries(${PROJECT_NAME}
  gtest
  painty
)

add_dependencies(${PROJECT_NAME}
  painty
)
