cmake_minimum_required(VERSION 3.10.2)

project(painty_test)


# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
  main.cpp
  vec_test.cpp
  mat_test.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${GTEST_INCLUDE_DIRS}>
)

target_link_libraries(${PROJECT_NAME}
  ${GTEST_LIBRARIES}
  pthread
  painty
)

add_dependencies(${PROJECT_NAME}
  painty
)
