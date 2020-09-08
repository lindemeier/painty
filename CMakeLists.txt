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

# 3rdParty
add_subdirectory(3rdParty/eigen)

# modules
add_subdirectory(painty/core)
add_subdirectory(painty/image)
add_subdirectory(painty/io)
add_subdirectory(painty/renderer)

# apps
option(BUILD_APPS "Build apps" ON)
if(BUILD_APPS)
  add_subdirectory(apps/painty_gui)
endif()
