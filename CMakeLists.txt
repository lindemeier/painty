cmake_minimum_required(VERSION 3.10.2)

project(painty_root)

# lib
add_subdirectory(src)

# test
option(RUN_TESTS "Build and run the tests" ON)
if(RUN_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# apps
option(BUILD_APPS "Build apps" ON)
if(BUILD_APPS)
    add_subdirectory(apps/painty_gui)
endif()
