#include <map>
#include <algorithm>
#include <memory>
#include <vector>

#include "Market.h"
#include "MarketPlayer.h"
#include "LimitOrderBook.h"

Market::Market(unsigned int numTraders) {
    marketPlayers.reserve(numTraders);
    for (size_t i {0}; i < numTraders; ++i)
        marketPlayers.emplace_back();
}

// need to figure out in what order to place the orders and
void Market::simulateMarket(unsigned int numSteps) {
    std::map<double, std::shared_ptr<Order>> ordersTimestamped{};
    for (size_t i = 0; i < numSteps; i++)
    {
        // generate orders for each trader and store them in a map with the timestamp as the key
        for (MarketPlayer & trader: marketPlayers)
            ordersTimestamped[trader.generateTimestamp()] = trader.trade(newsGenerator(), limitOrderBook); // need to find a way with for dealing with duplicate timestamps

        // process the orders in order of timestamp
        for (auto& timestampedOrder : ordersTimestamped)
        {
            limitOrderBook.addOrder(timestampedOrder.second);
        }

        limitOrderBook.matchOrders();
        //we need to save market data to memory after each iteration
        ordersTimestamped.clear();
    }
}

double Market::printMarketData() const {
    return limitOrderBook.get_mid_price();
}
