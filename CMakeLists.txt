cmake_minimum_required(VERSION 3.21)

project(Phoenix
    VERSION 0.1.0
    DESCRIPTION "Project Phoenix beta"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PHOENIX_BUILD_TESTS "Build Phoenix tests" ON)

find_package(assimp CONFIG REQUIRED)

add_subdirectory(apps/editor)

if(PHOENIX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
