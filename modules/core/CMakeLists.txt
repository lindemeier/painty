cmake_minimum_required(VERSION 3.10.2)

project(paintyCore)

# eigen
find_package (Eigen3 3.3 REQUIRED NO_MODULE)

add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME}
  INTERFACE ${PROJECT_SOURCE_DIR}/include/painty/core/color.h
  INTERFACE ${PROJECT_SOURCE_DIR}/include/painty/core/math.h
  INTERFACE ${PROJECT_SOURCE_DIR}/include/painty/core/kubelka_munk.h
  INTERFACE ${PROJECT_SOURCE_DIR}/include/painty/core/spline.h
  INTERFACE ${PROJECT_SOURCE_DIR}/include/painty/core/types.h
  INTERFACE ${PROJECT_SOURCE_DIR}/include/painty/core/vec.h
)

target_include_directories(${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
  INTERFACE $<INSTALL_INTERFACE:include/>
)

target_link_libraries(${PROJECT_NAME} INTERFACE
    Eigen3::Eigen
)

if(RUN_TESTS)
  add_subdirectory(test)
endif()
