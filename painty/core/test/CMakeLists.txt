cmake_minimum_required(VERSION 3.10.2)

project(paintyCoreTest)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/MathTest.cpp
    ${PROJECT_SOURCE_DIR}/src/KubelkaMunkTest.cpp
    ${PROJECT_SOURCE_DIR}/src/SplineTest.cpp
    ${PROJECT_SOURCE_DIR}/src/VecTest.cpp
  )

target_link_libraries(${PROJECT_NAME}
  gtest
  paintyCore
)
add_dependencies(${PROJECT_NAME}
  paintyCore
)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
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
