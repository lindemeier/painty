cmake_minimum_required(VERSION 3.10.2)

project(paintyImage)

find_package(OpenCV REQUIRED core imgproc)

add_library(${PROJECT_NAME} STATIC
  ${PROJECT_SOURCE_DIR}/src/ColorExtraction.cxx
  ${PROJECT_SOURCE_DIR}/src/Convolution.cxx
  ${PROJECT_SOURCE_DIR}/src/FlowBasedDoG.cxx
  ${PROJECT_SOURCE_DIR}/src/TextureWarp.cxx
  ${PROJECT_SOURCE_DIR}/src/EdgeTangentFlow.cxx
  ${PROJECT_SOURCE_DIR}/src/Superpixel.cxx
)

target_include_directories(${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
  PUBLIC $<INSTALL_INTERFACE:include/>
  PUBLIC ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC paintyCore
  ${OpenCV_LIBS}
)

add_dependencies(${PROJECT_NAME}
  paintyCore
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
