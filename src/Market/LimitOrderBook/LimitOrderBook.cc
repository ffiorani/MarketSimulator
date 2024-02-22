#include <memory>
#include <iostream>

#include "LimitOrderBook.h"
#include "MarketPlayer.h"

/***************************** ORDER METHODS *****************************/

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



/***************************** LIMIT ORDER BOOK METHODS *****************************/

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

double LimitOrderBook::get_total_ask_volume() const {
    return ask_prices.getTotalVolume();
}

double LimitOrderBook::get_total_bid_volume() const {
    return bid_prices.getTotalVolume();
}

double LimitOrderBook::get_total_volume() const {
    return ask_prices.getTotalVolume() + bid_prices.getTotalVolume();
}

double LimitOrderBook::getVolumeImbalance() const {
    return ask_prices.getTotalVolume() - bid_prices.getTotalVolume();
}

double LimitOrderBook::getSpread() const {
    if (ask_prices.empty() || bid_prices.empty())
        return 0.;

    return get_ask() - get_bid();
}

int LimitOrderBook::getNumActiveOrders() const {
    return activeOrders.size() + ask_prices.size() + bid_prices.size();
}




// helper functions for matchOrders

void LimitOrderBook::placeLimitOrder(std::shared_ptr<Order> limitOrderPtr) {

        if (limitOrderPtr -> isBuy) { // perhaps the previous implementation should be part of match Orders: otherwise limit orders will have priority over market orders!!
            bid_prices.push(limitOrderPtr);
        } else {
            ask_prices.push(limitOrderPtr);
        }
}

void LimitOrderBook::addOrder(std::shared_ptr<Order> orderPtr) {
    activeOrders.push_back(orderPtr);
}


void LimitOrderBook::matchOrders() {
    if (activeOrders.empty())
        return ;

    std::list<std::shared_ptr<Order>> RemainingActiveMarketOrders {};
    // compute vwap at each step
    double vwap {};
    double totalTimeStepVolume {};

    // iterate through all the active orders;
    for (auto orderPtr: activeOrders)
    {
        assert(orderPtr != nullptr);

        if (auto currTrader = ((orderPtr) -> trader).lock()) {
            std::pair<double, double> volumeAndTransaction {};

            if ((orderPtr) -> isBuy)
            {
                volumeAndTransaction = walkTheBook(orderPtr, ask_prices);
            } else {
                volumeAndTransaction = walkTheBook(orderPtr, bid_prices);
            }

            double unmatchedVolume {volumeAndTransaction.first};
            double currTransaction {volumeAndTransaction.second};


            // update VWAP price
            totalTimeStepVolume += (orderPtr) -> volume - unmatchedVolume;
            vwap += currTransaction;

            // update VWAP and prices, as they'll be used in the price change
            if (totalTimeStepVolume > 0)
                updateVWAP(vwap / totalTimeStepVolume);


            // update the price of the limit order book
            updateBookPrices();

            // if the order is not fully matched, update the order and append it to the limit order book
            if (unmatchedVolume > 0) {
                // update volume of the order
                (orderPtr) -> updateOrderVolume(unmatchedVolume);

                // append order to the active orders of the trader
                currTrader -> addOrder(orderPtr);

                // update fictitious portfolio
                currTrader -> updatePortfolioIfOrdersMatched((orderPtr) -> price, unmatchedVolume, !((orderPtr) -> isBuy));

                // if limit order, append to limit order book
                if ((orderPtr) -> isLimit)
                {
                    placeLimitOrder(orderPtr);

                } else { // else append to the activeOrders
                    RemainingActiveMarketOrders.push_back(orderPtr);
                }
            }
        } else {
            // if the trader is no longer in the market, delete the order
            deleteOrder(orderPtr);
        }
    }


    // update active orders
    activeOrders.clear();
    activeOrders = RemainingActiveMarketOrders;
}


void LimitOrderBook::updateVWAP(double newVWAP) {
    vwap = newVWAP;
}

void LimitOrderBook::updateBookPrices() {
    double priceChange {(get_VWAP() - get_mid_price()) * PRICE_CHANGE_FACTOR +
                        getVolumeImbalance() * VOLUME_PRICE_CHANGE_FACTOR};

    ask_prices.updatePrices(priceChange);
    bid_prices.updatePrices(priceChange);
}


// currently not needed: we delete orders from the book by popping them from the queue;
// however might be used by market players
void LimitOrderBook::deleteLimitOrder(std::shared_ptr<Order> orderPtr) {
    if (orderPtr -> isBuy)
        bid_prices.remove(orderPtr);
    else
        ask_prices.remove(orderPtr);
}

void LimitOrderBook::deleteMarketOrder(std::shared_ptr<Order> orderPtr) {
    activeOrders.remove(orderPtr);
}

void LimitOrderBook::deleteOrder(std::shared_ptr<Order> orderPtr) {
    if (orderPtr -> isLimit)
        deleteLimitOrder(orderPtr);
    else
        deleteMarketOrder(orderPtr);
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
