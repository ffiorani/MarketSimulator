add_library(MarketPlayerLib STATIC MarketPlayer.cc)
target_include_directories(MarketPlayerLib PUBLIC "./")
target_link_libraries(MarketPlayerLib PUBLIC LimitOrderBookLib)
