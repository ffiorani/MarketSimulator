#include <memory>
#include <iostream>

#include "LimitOrderBook.h"
#include "MarketPlayer.h"

// Implementing Order methods

Order::Order(double price, double volume, bool isBuy, bool isLimit, std::shared_ptr<MarketPlayer> trader) :
                        price {price}, volume {volume}, isBuy {isBuy}, isLimit {isLimit}, trader{trader} {}

bool Order::operator<(const Order & other) const {
    if (this -> price < other.price) {
        return true;
    }

    if (this -> price == other.price && this -> volume < other.volume )
    {
        return true;
    }

    return false;
}

bool Order::operator>(const Order & other) const {
    if (this -> price > other.price) {
        return true;
    }

    if (this -> price == other.price && this -> volume > other.volume )
    {
        return true;
    }

    return false;
}


void Order::updateOrderVolume(double newVolume) {
    volume = newVolume;
}

void Order::updateOrderPrice(double newPrice) {
    price = newPrice;
}

void Order::addToCurrentPrice(double volumeAdjustedOutcome) {
    double tempPrice {price};
    tempPrice += volumeAdjustedOutcome;

    if (tempPrice > UPPER_PRICE_LIMIT)
    {
        price = UPPER_PRICE_LIMIT;
        return ;
    }

    if (tempPrice < LOWER_PRICE_LIMIT)
    {
        price = LOWER_PRICE_LIMIT;
        return ;
    }

    price = tempPrice;

}



// Implementing LimitOrderBook methods

bool LimitOrderBook::hasAskOrders() const {
    return !ask_prices.empty();
}

bool LimitOrderBook::hasBidOrders() const {
    return !bid_prices.empty();
}

double LimitOrderBook::get_ask() const {
    if (ask_prices.empty())
        return UPPER_PRICE_LIMIT;

    return ask_prices.top() -> price;
}

double LimitOrderBook::get_bid() const {
    if (bid_prices.empty())
        return LOWER_PRICE_LIMIT;

    return bid_prices.top() -> price;
}

double LimitOrderBook::get_mid_price() const {
    if (ask_prices.empty() && bid_prices.empty())
        return INITIAL_PRICE; // defined in the header file

    if (ask_prices.empty())
        return LOWER_PRICE_LIMIT;

    if (bid_prices.empty())
        return UPPER_PRICE_LIMIT;

    return (ask_prices.top() -> price + bid_prices.top() -> price) / 2.;
}

double LimitOrderBook::get_VWAP() const {
    return vwap;
}

double LimitOrderBook::get_total_volume() const {
    return totalVolume;
}

double LimitOrderBook::getSpread() const {
    return ask_prices.top() -> price - bid_prices.top() -> price;
}

int LimitOrderBook::getNumActiveOrders() const {
    return activeOrders.size() + ask_prices.size() + bid_prices.size();
}

void LimitOrderBook::placeLimitOrder(std::shared_ptr<Order> limitOrderPtr) {

        if (limitOrderPtr -> isBuy) { // perhaps the previous implementation should be part of match Orders: otherwise limit orders will have priority over market orders!!
            bid_prices.push(limitOrderPtr);
        } else {
            ask_prices.push(limitOrderPtr);
        }
}

// this is duplicated by the addOrder method
void LimitOrderBook::appendMarketOrder(std::shared_ptr<Order> marketOrderPtr) {
    activeOrders.push_back(marketOrderPtr);
}

// helper functions for matchOrders

void LimitOrderBook::addOrder(std::shared_ptr<Order> orderPtr) {
    activeOrders.push_back(orderPtr);
}

void LimitOrderBook::matchOrders() {
    if (activeOrders.empty())
        return ;

    std::list<std::shared_ptr<Order>> RemainingActiveMarketOrders {};
    // compute vwap at each step
    double vwap {};

    // getting pointers to the top of the queues, if they exists

    // iterate through all the active orders;
    auto it {activeOrders.begin()};
    for (; it != activeOrders.end(); it++)
    {
        std::pair<double, double> volumeAndTransaction {};
        if ((*it) -> isBuy)
        {
            volumeAndTransaction = walkTheBook(*it, ask_prices);
        } else {
            volumeAndTransaction = walkTheBook(*it, bid_prices);
        }

        double currVolume {volumeAndTransaction.first};
        double currTransaction {volumeAndTransaction.second};

        // update the price of the limit order book
        // updateBookPrices(currTransaction, currVolume);

        // update total volume
        totalVolume += currVolume;

        // update VWAP price
        vwap += currTransaction;

        // if the order is not fully matched, update the order and append it to the limit order book
        if (currVolume > 0) {
            // update volume of the order
            (*it) -> updateOrderVolume(currVolume);

            // if limit order, append to limit order book
            if ((*it) -> isLimit)
            {
                placeLimitOrder(*it);

            } else { // else append to the activeOrders
                RemainingActiveMarketOrders.push_front(*it);
            }
        }
    }

    // update VWAP and prices
    updateVWAP(vwap);

    // update active orders
    activeOrders.clear();
    activeOrders = RemainingActiveMarketOrders;
}


void LimitOrderBook::updateVWAP(double newVWAP) {
    vwap = newVWAP;
}

// should we also make it depend more directly on the difference between total ask volume and total bid volume?
void LimitOrderBook::updateBookPrices(double transaction, double volume){
    double volumeAdjustedDifferential {transaction / volume - get_mid_price()};
    ask_prices.updatePrices(volumeAdjustedDifferential);
    bid_prices.updatePrices(volumeAdjustedDifferential);
}


// currently not needed: we delete orders from the book by popping them from the queue;
// however might be used by market players
void LimitOrderBook::deleteLimitOrder(std::shared_ptr<Order> orderPtr) {
    if (orderPtr -> isBuy)
        bid_prices.remove(orderPtr);
    else
        ask_prices.remove(orderPtr);
}

std::ostream & operator<<(std::ostream & os, const LimitOrderBook & book) {
    os << "Ask orders: ";
    os << "**********************************\n";
    os << book.ask_prices << std::endl;
    os << "**********************************\n";

    os << "Bid orders: ";
    os << "**********************************\n";
    os << book.bid_prices << std::endl;
    os << "**********************************\n";

    os << "Unmatched orders: ";
    for (auto it = book.activeOrders.cbegin(); it != book.activeOrders.cend(); it++)
        os << (*it) -> price << " ";
    os << "\n";

    return os;
}
