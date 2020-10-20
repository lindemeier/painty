cmake_minimum_required(VERSION 3.10.2)

project(paintyCore)

# eigen
find_package (Eigen3 REQUIRED NO_MODULE)

add_library(${PROJECT_NAME} STATIC
  ${PROJECT_SOURCE_DIR}/src/ThreadPool.cxx
  ${PROJECT_SOURCE_DIR}/src/Timer.cxx
)

target_include_directories(${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
  PUBLIC $<INSTALL_INTERFACE:include/>
)

target_link_libraries(${PROJECT_NAME}
  Eigen3::Eigen
)

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
