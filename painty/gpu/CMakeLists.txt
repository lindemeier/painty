cmake_minimum_required(VERSION 3.10.2)

project(paintyGpu)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
  INTERFACE $<INSTALL_INTERFACE:include/>
)

target_link_libraries(${PROJECT_NAME}
 INTERFACE paintyImage
 INTERFACE prgl
)

add_dependencies(${PROJECT_NAME}
  paintyImage
  prgl
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
  target_compile_options(${PROJECT_NAME} INTERFACE -Wall -Weverything -Wno-c++98-compat -Wno-padded -Wno-documentation -Werror -Wno-global-constructors)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  target_compile_options(${PROJECT_NAME} INTERFACE -Wall -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  target_compile_options(${PROJECT_NAME} INTERFACE /W4 /WX)
endif()
if(RUN_TESTS)
  add_subdirectory(test)
endif()
