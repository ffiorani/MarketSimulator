add_library(MarketPlayerLib STATIC MarketPlayer.cc)

set(INCLUDE_DIRS "./" "../LimitOrderBook")

target_include_directories(MarketPlayerLib PUBLIC INCLUDE_DIRS)

target_link_libraries(MarketPlayerLib PUBLIC LimitOrderBookLib)
