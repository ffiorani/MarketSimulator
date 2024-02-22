#include <map>
#include <algorithm>
#include <memory>
#include <vector>

#include "Market.h"
#include "MarketPlayer.h"
#include "LimitOrderBook.h"

Market::Market(unsigned int numTraders) {
    marketPlayers.reserve(numTraders);
    for (size_t i {0}; i < numTraders; ++i) {
        auto traderPtr = std::make_shared<MarketPlayer>();
        marketPlayers.push_back(traderPtr);
    }
}

unsigned int Market::getNumTraders() const {
    return marketPlayers.size();
}

std::string Market::printMarketSummary() const {
    std::string summary;
    for (auto traderPtr: marketPlayers)
    {
        summary += "Trader " + std::to_string(traderPtr -> getId()) + " has " + std::to_string(traderPtr -> getCash()) +
                   " in cash and " + std::to_string(traderPtr -> getShares()) + " in shares\n";
    }
    return summary;
}

std::vector<double> Market::getCashDistribution() const {
    std::vector<double> cashDistribution;
    cashDistribution.reserve(marketPlayers.size());
    for (auto traderPtr: marketPlayers)
    {
        cashDistribution.push_back(traderPtr -> getCash());
    }
    std::sort(cashDistribution.begin(), cashDistribution.end());
    return cashDistribution;
}

std::vector<double> Market::getSharesDistribution() const {
    std::vector<double> sharesDistribution;
    sharesDistribution.reserve(marketPlayers.size());
    for (auto traderPtr: marketPlayers)
    {
        sharesDistribution.push_back(traderPtr -> getShares());
    }
    std::sort(sharesDistribution.begin(), sharesDistribution.end());
    return sharesDistribution;
}

std::vector<double> Market::getWealthDistribution() const {
    std::vector<double> wealthDistribution;
    wealthDistribution.reserve(marketPlayers.size());
    for (auto traderPtr: marketPlayers)
    {
        wealthDistribution.push_back(traderPtr -> getTotalWealth(this -> limitOrderBook));
    }
    std::sort(wealthDistribution.begin(), wealthDistribution.end());
    return wealthDistribution;
}

// still to implement
// perhaps could implement a public method in LimitOrderBook that deletes the order from the limit order book
// and a public method in the MarketPlayer class that deletes the order from the market player. Perhaps it would be
// better to have the market player store their orders in a priority queue or a binary tree,
// so that they can delete the order in O(log n)
// maybe even better: the deletion could be a function which is friends with both the MarketPlayer and the LimitOrderBook
// which makes more sense as it is either the marketPlayer or the LimitOrderBook which delete orders,
// which are shared between the two classes
void deleteOrder(std::shared_ptr<Order> OrderPtr) {
    // need to find a way to delete the order from the limit order book
    // need to find a way to delete the order from the market player
}

// need to figure out in what order to place the orders and
void Market::simulateMarket(unsigned int numSteps, std::vector<double> & priceHistory) {
    std::map<double, std::shared_ptr<Order>> ordersTimestamped{};
    priceHistory.reserve(numSteps);

    for (size_t i = 0; i < numSteps; i++)
    {
        // generate orders for each trader and store them in a map with the timestamp as the key
        for (auto traderPtr: marketPlayers)
        {
            auto orderPtr = trade(newsGenerator(), limitOrderBook, traderPtr);
            if (orderPtr != nullptr)
                ordersTimestamped.insert({traderPtr -> generateTimestamp(), orderPtr}); // need to find a way with for dealing with duplicate timestamps
        }

        // process the orders in order of timestamp
        for (auto & timestampedOrder : ordersTimestamped)
        {
            limitOrderBook.addOrder(timestampedOrder.second);
        }

        limitOrderBook.matchOrders();
        //we need to save market data to memory after each iteration
        ordersTimestamped.clear();
        priceHistory.push_back(limitOrderBook.get_mid_price());
    }
}

std::ostream & operator<<(std::ostream & os, const Market & market) {
    os << market.limitOrderBook;
    return os;
}
