add_executable(Executable main.cc)

set(LIBRARIES my_lib LimitOrderBookLib MarketPlayerLib MarketLib)

target_link_libraries(Executable PUBLIC LIBRARIES)
