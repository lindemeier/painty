cmake_minimum_required(VERSION 3.10.2)

project(painty_root)

# test
option(RUN_TESTS "Build and run the tests" ON)
if(RUN_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()


# nlohmann json
add_subdirectory(3rdParty/json)

# lib
add_subdirectory(src)
