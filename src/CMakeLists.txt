cmake_minimum_required(VERSION 3.10.2)

project(painty)

add_library(${PROJECT_NAME} STATIC
    image_io.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 14)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

target_include_directories(${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
    PUBLIC $<INSTALL_INTERFACE:include/>
)

# target_link_libraries(${PROJECT_NAME})
