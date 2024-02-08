
add_library(MarketPlayerLib STATIC MarketPlayer.h MarketPlayer.cc)

set(INCLUDE_DIRS "./")

target_include_directories(MarketPlayerLib PUBLIC ${INCLUDE_DIRS})

target_link_libraries(MarketPlayerLib LimitOrderBookLib)
