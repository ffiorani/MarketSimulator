cmake_minimum_required(VERSION 3.22)

project(Market_Simulator VERSION 1.0.0 LANGUAGES C CXX)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_TESTING "Enable Test Builds" ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
include(Docs)
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG v2.13.0
    GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(Catch2)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(app)



set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
