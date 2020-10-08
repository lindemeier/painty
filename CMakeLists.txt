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

set(CXXOPTS_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(CXXOPTS_BUILD_TESTS OFF CACHE INTERNAL "")
set(CXXOPTS_ENABLE_INSTALL OFF CACHE INTERNAL "")
FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(cxxopts)
#

set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.9.1
)
FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(
  prgl
  GIT_REPOSITORY https://github.com/lindemeier/prgl.git
  GIT_TAG        develop
)
FetchContent_MakeAvailable(prgl)


# modules
add_subdirectory(painty/core)
add_subdirectory(painty/image)
add_subdirectory(painty/gpu)
add_subdirectory(painty/io)
add_subdirectory(painty/mixer)
add_subdirectory(painty/renderer)
add_subdirectory(painty/sbr)

# apps
option(BUILD_APPS "Build apps" ON)
if(BUILD_APPS)
  add_subdirectory(apps/painty_gui)
  add_subdirectory(apps/palette_extraction)
  add_subdirectory(apps/sbr_painter)
endif()
