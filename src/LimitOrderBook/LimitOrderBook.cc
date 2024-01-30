#include <memory>

#include "LimitOrderBook.h"
#include "MarketPlayer.h"

// Implementing Order methods

Order::Order(double price, double volume, bool isBuy, bool isLimit, std::shared_ptr<MarketPlayer> trader) :
                        price {price}, volume {volume}, isBuy {isBuy}, isLimit {isLimit}, trader{trader} {}

bool Order::operator<(const Order & other) {
    if (this -> price < other.price) {
        return true;
    }

    if (this -> price == other.price && this -> volume < other.volume )
    {
        return true;
    }

    return false;
}


void Order::updateOrderVolume(double newVolume) {
    volume = newVolume;
}

void Order::updateOrderPrice(double newPrice) {
    volume = newPrice;
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

double LimitOrderBook::get_ask() const {
    if (ask_prices.empty())
        return UPPER_PRICE_LIMIT;

    return ask_prices.top() -> price;
}

double LimitOrderBook::get_bid() const {
    if (ask_prices.empty())
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

void LimitOrderBook::placeLimitOrder(std::shared_ptr<Order> limitOrderPtr) {

        if (limitOrderPtr -> isBuy) { // perhaps the previous implementation should be part of match Orders: otherwise limit orders will have priority over market orders!!
            bid_prices.push(limitOrderPtr);
        } else {
            ask_prices.push(limitOrderPtr);
        }
}

void LimitOrderBook::appendMarketOrder(std::shared_ptr<Order> marketOrderPtr) {
    activeOrders.push_back(marketOrderPtr);
}


void LimitOrderBook::matchOrders() {
    if (activeOrders.empty())
        return ;

    // quantity that computes the price update; the price is updated after each time step
    double volumeAdjustedPriceTrack {};
    double currPrice {get_mid_price()};
    double tradeVolume {};
    double tradePrice {};
    // compute vwap at each step
    double vwap {};

    // getting pointers to the top of the queues, if they exists

    // iterate through all the active orders;
    auto it {activeOrders.begin()};
    for (; it != activeOrders.end(); it++)
    {
        double currVolume {(*it) -> volume};
        if ((*it) -> isBuy)
        {
        // check if there are any limit orders that would match this order; if so pick the best price
            if ((!ask_prices.empty()) && (ask_prices.top() -> price <= (*it) -> price))
            {
                tradeVolume = 0;
                tradePrice = 0;
                while (currVolume > 0 && !ask_prices.empty())
                {
                    // compute volume of the trade
                    if (ask_prices.top() -> volume >= currVolume) {
                        tradeVolume = ask_prices.top() -> volume;
                    } else {
                        tradeVolume = currVolume;
                    }

                    // compute price of the trade
                    tradePrice = ask_prices.top() -> price;

                    // update traders' portfolio
                    ask_prices.top() -> trader -> updatePortfolio(tradeVolume * ask_prices.top() -> price, tradeVolume);
                    ask_prices.top() -> trader -> updatePortfolio(tradeVolume * ask_prices.top() -> price, tradeVolume);


                    // delete/update the Limit order
                    currVolume -= ask_prices.top() -> volume;
                    if (currVolume > 0) {
                        ask_prices.top() -> trader -> deleteActiveOrder(ask_prices.top());
                        ask_prices.pop();
                    } else {
                        ask_prices.top() -> updateOrderVolume(ask_prices.top() -> volume - tradeVolume);
                        ask_prices.top() -> updateOrderPrice(ask_prices.top() -> price);

                    }
                }
            }
        }
        else // same procedure for ask orders: is there a way to not duplicate the code?
        {
        // check if there are any limit orders that would match this order; if so pick the best price
            if ((!bid_prices.empty()) && (bid_prices.top() -> price <= (*it) -> price))
            {
                tradeVolume = 0;
                tradePrice = 0;
                while (currVolume > 0 && !bid_prices.empty())
                {
                    // compute volume of the trade
                    if (bid_prices.top() -> volume >= currVolume) {
                        tradeVolume = bid_prices.top() -> volume;
                    } else {
                        tradeVolume = currVolume;
                    }

                    // compute price of the trade
                    tradePrice = bid_prices.top() -> price;

                    // update traders' portfolio
                    bid_prices.top() -> trader -> updatePortfolio(tradeVolume * bid_prices.top() -> price, tradeVolume);
                    bid_prices.top() -> trader -> updatePortfolio(tradeVolume * bid_prices.top() -> price, tradeVolume);


                    // delete/update the Limit order
                    currVolume -= bid_prices.top() -> volume;
                    if (currVolume > 0) {
                        bid_prices.top() -> trader -> deleteActiveOrder(bid_prices.top());
                        bid_prices.pop();
                    } else {
                        bid_prices.top() -> updateOrderVolume(bid_prices.top() -> volume - tradeVolume);
                        bid_prices.top() -> updateOrderPrice(bid_prices.top() -> price);
                    }
                }
            }
        }

        // update total volume
        totalVolume -= tradeVolume;
        totalVolume += currVolume;

        // update VWAP price
        vwap += tradePrice * tradeVolume;

        // update cumulative price change quantity
        volumeAdjustedPriceTrack +=  (tradePrice - ((*it) -> price)) * tradeVolume;

        if (currVolume > 0) {
            // update volume of the order
            (*it) -> updateOrderVolume(currVolume);

            // if limit order, append to limit order book
            if ((*it) -> isLimit)
            {
                if ((*it) -> isBuy)
                    bid_prices.push((*it));
                else
                    ask_prices.push((*it));

            } else { // else append to the activeOrders
                appendMarketOrder(*it);

                // update price adjustment to take into account outstanding market orders
                if ((*it) -> isBuy) {
                    volumeAdjustedPriceTrack += ((*it) -> price) * (currVolume) / totalVolume;
                } else {
                    volumeAdjustedPriceTrack -= ((*it) -> price) * (currVolume) / totalVolume;
                }
            }
        }
    }

    // update VWAP and prices
    updateVWAP(vwap);
}


void LimitOrderBook::updateVWAP(double newVWAP) {
    vwap = newVWAP;
}


void LimitOrderBook::updatePrice(double volumeAdjustedOutcome) {
    ask_prices.updatePrices(volumeAdjustedOutcome);
    bid_prices.updatePrices(volumeAdjustedOutcome);
}


// currently not needed: we delete orders from the book by popping them from the queue;
// however might be used by market players
void LimitOrderBook::deleteLimitOrder(std::shared_ptr<Order> orderPtr) {
    ask_prices.remove(orderPtr);
    bid_prices.remove(orderPtr);
}
