cmake_minimum_required(VERSION 3.10.2)

project(paintyGpu)

add_library(${PROJECT_NAME} STATIC
  ${PROJECT_SOURCE_DIR}/src/GpuTaskQueue.cxx
)

target_include_directories(${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
  PUBLIC $<INSTALL_INTERFACE:include/>
)

target_link_libraries(${PROJECT_NAME}
 PUBLIC paintyImage
 PUBLIC prgl
)

add_dependencies(${PROJECT_NAME}
  paintyImage
  prgl
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
  target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Weverything -Wno-c++98-compat -Wno-padded -Wno-documentation -Werror -Wno-global-constructors)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  target_compile_options(${PROJECT_NAME} PUBLIC /W4 /WX)
endif()
if(RUN_TESTS)
  add_subdirectory(test)
endif()
