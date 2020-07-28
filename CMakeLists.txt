cmake_minimum_required(VERSION 3.16.0)

include(FetchContent)

project(painty)

option(RUN_TESTS "Build and run the tests" ON)
if(RUN_TESTS)
    # get google test
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG        release-1.10.0
    )
    FetchContent_MakeAvailable(googletest)
    enable_testing()
endif()

# Color conversion lib
FetchContent_Declare(
  ColorConverter
  GIT_REPOSITORY https://github.com/lindemeier/ColorConverter.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(ColorConverter)

# modules
add_subdirectory(modules/core)
add_subdirectory(modules/image)
add_subdirectory(modules/io)
add_subdirectory(modules/renderer)

# apps
option(BUILD_APPS "Build apps" ON)
if(BUILD_APPS)
    add_subdirectory(apps/painty_gui)
endif()
