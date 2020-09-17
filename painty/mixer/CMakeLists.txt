cmake_minimum_required(VERSION 3.16.0)

project(paintyMixer)

# ceres solver library
find_package(Ceres REQUIRED)

include(FetchContent)
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

add_library(${PROJECT_NAME} STATIC
  ${PROJECT_SOURCE_DIR}/src/Palette.cxx
  ${PROJECT_SOURCE_DIR}/src/PaintCoeff.cxx
  ${PROJECT_SOURCE_DIR}/src/PaintMixer.cxx
  ${PROJECT_SOURCE_DIR}/src/Serialization.cxx
)

target_include_directories(${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
  PUBLIC $<BUILD_INTERFACE:${CERES_INCLUDE_DIRS}>
  PUBLIC $<INSTALL_INTERFACE:include/>
)

target_link_libraries(${PROJECT_NAME}
  paintyCore
  paintyImage
  ${CERES_LIBRARIES}
  nlohmann_json::nlohmann_json
)

add_dependencies(${PROJECT_NAME}
  paintyCore
  paintyImage
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
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


if(RUN_TESTS)
  add_subdirectory(test)
endif()
