include_directories(${CMAKE_SOURCE_DIR}/src/Market/)

# Add executable
add_executable(Executable main.cc)

# Link the executable with the library
target_link_libraries(Executable MarketLib)
