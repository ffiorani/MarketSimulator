cmake_minimum_required(VERSION 3.22)

project(Market_Simulator VERSION 1.0.0 LANGUAGES C CXX)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(app)
add_subdirectory(src/Market)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
