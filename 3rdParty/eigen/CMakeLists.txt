cmake_minimum_required(VERSION 3.14...3.18)

project(eigen LANGUAGES CXX)

include(FetchContent)

set(BUILD_TESTING OFF)
set(BUILD_EXAMPLES OFF)
FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/thomas.lindemeier/eigen.git
  GIT_TAG 3.3.7-patched
)
FetchContent_MakeAvailable(eigen)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
  target_compile_options(${PROJECT_NAME} INTERFACE -Wall -Wno-c++98-compat -Wno-padded -Wno-documentation -Werror -Wno-global-constructors
                        -Wno-old-style-cast -Wno-extra-semi-stmt -Wno-sign-conversion -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic -Wno-zero-as-null-pointer-constant
                        -Wno-reserved-id-macro -Wno-shadow -Wno-extra-semi -Wno-used-but-marked-unused -Wno-cast-qual -Wno-missing-noreturn -Wno-unused-template -Wno-deprecated -Wno-unused-parameter
                        )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  target_compile_options(${PROJECT_NAME} INTERFACE -Wall -Wno-c++98-compat -Wno-padded -Wno-documentation -Werror -Wno-global-constructors
                        -Wno-old-style-cast -Wno-extra-semi-stmt -Wno-sign-conversion -Wno-documentation-unknown-command -Wno-c++98-compat-pedantic -Wno-zero-as-null-pointer-constant
                        -Wno-reserved-id-macro -Wno-shadow -Wno-extra-semi -Wno-used-but-marked-unused -Wno-cast-qual -Wno-missing-noreturn -Wno-unused-template -Wno-deprecated -Wno-unused-parameter
  )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  target_compile_options(${PROJECT_NAME} INTERFACE /W4 /WX)
endif()
