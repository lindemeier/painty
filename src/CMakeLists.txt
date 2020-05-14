cmake_minimum_required(VERSION 3.10.2)

project(painty)

# image io
find_package(PNG REQUIRED)

add_library(${PROJECT_NAME} STATIC
    brush_stroke_sample.cpp
    image_io.cpp
    texture_warp.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

target_include_directories(${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
    PUBLIC $<INSTALL_INTERFACE:include/>
    PUBLIC $<BUILD_INTERFACE:${PNG_INCLUDE_DIR}>
    PRIVATE ${ColorConverter_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${PNG_LIBRARY}
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Weverything -Wno-c++98-compat -Wno-padded -Wno-documentation -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
endif()
