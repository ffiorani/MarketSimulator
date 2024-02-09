#include_directories(${CMAKE_SOURCE_DIR}/src/Market/)
set(TEST_MAIN "MarketPlayerTests")
set(TEST_SOURCES "MarketPlayerTests.cc")
set(TEST_INCLUDES "./")

add_executable(${TEST_MAIN} ${TEST_SOURCES})
target_include_directories(${TEST_MAIN} PUBLIC ${TEST_INCLUDES})
target_link_libraries(${TEST_MAIN} MarketPlayerLib Catch2::Catch2)
