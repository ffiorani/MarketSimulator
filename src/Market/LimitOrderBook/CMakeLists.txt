add_library(LimitOrderBookLib STATIC LimitOrderBook.h LimitOrderBook.cc)

set(INCLUDE_DIRS "./")

target_include_directories(LimitOrderBookLib PUBLIC ${INCLUDE_DIRS})

target_link_libraries(LimitOrderBookLib MarketPlayerLib)
