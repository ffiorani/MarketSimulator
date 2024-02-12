include_directories(${CMAKE_SOURCE_DIR}/src/Market/)
set(TEST_MAIN "MarketTests")
set(TEST_SOURCES "MarketTests.cc")
set(TEST_INCLUDES "./")

add_executable(${TEST_MAIN} ${TEST_SOURCES})
target_include_directories(${TEST_MAIN} PUBLIC ${TEST_INCLUDES})
target_link_libraries(${TEST_MAIN} MarketLib LimitOrderBookLib MarketPlayerLib Catch2::Catch2)
