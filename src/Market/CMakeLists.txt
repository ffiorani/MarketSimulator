add_subdirectory(LimitOrderBook)
add_subdirectory(MarketPlayer)

add_library(MarketLib STATIC Market.cc)

target_link_libraries(MarketLib PUBLIC LimitOrderBookLib MarketPlayerLib)
