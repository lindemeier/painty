cmake_minimum_required(VERSION 3.10.2)

project(painty)

find_package(OpenCV 4.2.0 REQUIRED
    core
    imgproc
    imgcodecs
    highgui
)

add_library(${PROJECT_NAME} STATIC
    io.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 14)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

target_include_directories(${PROJECT_NAME}
        PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
        PUBLIC $<INSTALL_INTERFACE:include/>
        PUBLIC ${OpenCV_INCLUDE_DIRS}
        )

target_link_libraries(${PROJECT_NAME}
        ${TIFF_LIBRARIES}
        ${TIFFXX_LIBRARIES}
        ${OpenCV_LIBS}
        ${Boost_LIBRARIES}
        AlgLib
        )
