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

# modules
add_subdirectory(painty/core)
add_subdirectory(painty/image)
add_subdirectory(painty/io)
add_subdirectory(painty/mixer)
add_subdirectory(painty/renderer)
add_subdirectory(painty/sbr)

# apps
option(BUILD_APPS "Build apps" ON)
if(BUILD_APPS)
  add_subdirectory(apps/painty_gui)
  add_subdirectory(apps/palette_extraction)
endif()
