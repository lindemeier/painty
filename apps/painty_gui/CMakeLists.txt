cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(painty_gui)

# Qt
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
# Find the Qt5Core library
find_package(Qt5Core)
#find_package(Qt5Xml)
find_package(Qt5Widgets)
#find_package(Qt5Network)

#
add_executable(${PROJECT_NAME}
    src/digital_paint.cpp
    src/digital_canvas.h
    src/digital_canvas.cpp
    src/digital_paint_main_window.h
    src/digital_paint_main_window.cpp
    src/KM_visualize_widget.h
    src/KM_visualize_widget.cpp
    )

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

target_link_libraries(${PROJECT_NAME}
    painty
    Qt5::Core
    Qt5::Widgets
    )
add_dependencies(${PROJECT_NAME}
    painty
    Qt5::Core
    Qt5::Widgets
    )

target_include_directories(${PROJECT_NAME}
    PRIVATE src/
    )
