
add_library(MarketPlayerLib STATIC MarketPlayer.h MarketPlayer.cc)
set(LIBRARY_INCLUDES "./")

target_include_directories(MarketPlayerLib PUBLIC ${LIBRARY_INCLUDES})

target_link_libraries(MarketPlayerLib LimitOrderBookLib)
