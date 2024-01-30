#include <functional>
#include <algorithm>

#include "Market.h"
#include "MarketPlayer.h"
#include "LimitOrderBook.h"

Market::Market(unsigned int numTraders) : marketPlayers() {
    marketPlayers.reserve(numTraders);
    for (size_t i {0}; i < numTraders; ++i)
        marketPlayers.emplace(MarketPlayer());
}

// need to figure out in what order to place the orders and
void Market::simulateMarket(unsigned int numSteps) {
    std::map<double, Order> ordersTimestamped;
    for (size_t i = 0; i < count; i++)
    {
        for (MarketPlayer trader: marketPlayers)
            ordersTimestamped[trader.generateTimestamp()] = trader.trade();

        auto it {ordersTimestamped.begin()};
        for (; it < ordersTimestamped.end(); it++)
        {
            // no rule for breaking ties between timestamps, hopefully shouldn't occur often
            this -> processOrder(it -> second);
        }
        this -> limitOrderBook.matchOrders();
        //we need to save market data to memory after each iteration
        ordersTimestamped.clear();
    }
}

void Market::processOrder(Order & order) {
    // if placing order of opposite nature of the active orders, delete all active orders
    if ((!(order.trader -> activeOrders).empty()) && ((order.trader -> activeOrders.at(0)) -> isBuy != order.isBuy)) {
            for (std::shared_ptr<Order> & activeOrder: activeOrders)
                this -> limitOrderBook.deleteActiveOrder(activeOrder);

            (order.trader) -> deleteActiveOrders();
    }

    std::shared_ptr<Order> orderPtr = std::make_shared<Order>(order);
    order.trader -> activeOrders.push_back(orderPtr);
    // dealing separately with market orders and limit orders
    if (order.isLimit) { // limit order
        this -> limitOrderBook.placeLimitOrder(orderPtr);
    } else { // market order
        this -> limitOrderBook.appendMarketOrder(orderPtr);
        }
    }
}
