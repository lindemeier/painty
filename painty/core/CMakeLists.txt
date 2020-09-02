cmake_minimum_required(VERSION 3.10.2)

project(paintyCore)

add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME}
  INTERFACE ${PROJECT_SOURCE_DIR}/Color.h
  INTERFACE ${PROJECT_SOURCE_DIR}/Math.h
  INTERFACE ${PROJECT_SOURCE_DIR}/KubelkaMunk.h
  INTERFACE ${PROJECT_SOURCE_DIR}/Spline.h
  INTERFACE ${PROJECT_SOURCE_DIR}/Types.h
  INTERFACE ${PROJECT_SOURCE_DIR}/Vec.h
)

target_include_directories(${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
  INTERFACE $<INSTALL_INTERFACE:include/>
)

target_link_libraries(${PROJECT_NAME} INTERFACE
  eigen
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
  target_compile_options(${PROJECT_NAME} INTERFACE -Wall -Weverything -Wno-c++98-compat -Wno-padded -Wno-documentation -Werror -Wno-global-constructors
                        )
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