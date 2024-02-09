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
        marketPlayers.push_back(std::make_shared<MarketPlayer>());
}

unsigned int Market::getNumTraders() const {
    return marketPlayers.size();
}

std::string Market::printMarketSummary() const {
    std::string summary;
    for (auto traderPtr: marketPlayers)
    {
        summary += "Trader " + std::to_string(traderPtr -> getId()) + " has " + std::to_string(traderPtr -> getCash()) + " in cash and " + std::to_string(traderPtr -> getShares()) + " in shares\n";
    }
    return summary;
}

// need to figure out in what order to place the orders and
void Market::simulateMarket(unsigned int numSteps) {
    std::map<double, std::shared_ptr<Order>> ordersTimestamped{};
    for (size_t i = 0; i < numSteps; i++)
    {
        // generate orders for each trader and store them in a map with the timestamp as the key
        for (auto traderPtr: marketPlayers)
            ordersTimestamped.insert({traderPtr -> generateTimestamp(), trade(newsGenerator(), limitOrderBook, traderPtr)}); // need to find a way with for dealing with duplicate timestamps

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

std::ostream & operator<<(std::ostream & os, const Market & market) {
    os << market.limitOrderBook;
    return os;
}
