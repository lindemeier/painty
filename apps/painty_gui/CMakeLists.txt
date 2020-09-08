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
    src/DigitalPaint.cpp
    src/DigitalCanvas.h
    src/DigitalCanvas.cpp
    src/DigitalPaintMainWindow.h
    src/DigitalPaintMainWindow.cpp
    src/KMVisualizeWidget.h
    src/KMVisualizeWidget.cpp
    )

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

target_link_libraries(${PROJECT_NAME}
    paintyRenderer
    Qt5::Core
    Qt5::Widgets
    )
add_dependencies(${PROJECT_NAME}
    paintyRenderer
    Qt5::Core
    Qt5::Widgets
    )

target_include_directories(${PROJECT_NAME}
    PRIVATE src/
    )

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Weverything -Wno-c++98-compat -Wno-padded -Wno-documentation -Werror -Wno-global-constructors -Wno-redundant-parens -Wno-extra-semi-stmt)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
endif()
