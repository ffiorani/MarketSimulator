add_subdirectory(LimitOrderBook)
add_subdirectory(MarketPlayer)

set(LIBRARY_INCLUDES "./")
add_library(MarketLib STATIC Market.cc)

target_link_directories(MarketLib PUBLIC ${LIBRARY_INCLUDES})
target_link_libraries(MarketLib PUBLIC LimitOrderBookLib MarketPlayerLib)
