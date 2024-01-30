add_library(MarketLib STATIC Market.cc)
target_include_directories(MarketLib PUBLIC "./")
target_link_libraries(MarketLib PUBLIC LimitOrderBookLib MarketPlayerLib)
