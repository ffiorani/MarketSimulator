#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <memory>
#include <list>
#include <cassert>
#include <iostream>

#include "constants.cc"
#include "MarketPlayer.h"




/***************************** ORDER STRUCT ******************************/
/**
 * @brief
 * The Order struct is used to represent the orders in the LimitOrderBook.
 * It contains the price, volume, whether it is a buy or sell order, whether it is a limit or market order.
 * It also contains a weak pointer to the trader who placed the order.
 */
struct Order
{
double price;
double volume;

bool isBuy;
bool isLimit;
std::weak_ptr<MarketPlayer> trader;

/**
 * @brief Construct a new Order object
 * takes in @param price, @param volume, whether it is a buy order, whether it is a limit order,
 * and a shared pointer to the trader who placed the order
 */
Order(double, double, bool, bool, std::shared_ptr<MarketPlayer>);

/**
 * @brief Replaces the volume of the order with the @param newVolume
 */
void updateOrderVolume(double);

/**
 * @brief Replaces the price of the order with the @param newPrice
 */
void updateOrderPrice(double);

/**
 * @brief Updates the price of the order by adding the input parameter to the current price
 * checks whether the price is exceeding the upper or lower price limit
 * and sets the price to the limit if it does
 * used in the custom_priority_queue updatePrices method
 *
 */
void addToCurrentPrice(double);

/**
 * @brief Orders the first by price, then by volume
 *
 */
bool operator<(const Order &) const;

/**
 * @brief Orders the first by price, then by volume
 *
 */
bool operator>(const Order &) const;

};



/***************************** CUSTOM PRIORITY QUEUE ******************************/
/**
 * @brief struct that allows to apply STL methods to containers of strings
 * used in particular by the custom_priority_queue struct and the LimitOrderBook::activeOrders vectors
 *
 * @tparam T
 * Use case is mainly for shared pointers to orders
 */
template<class T>
struct PtrLess
    : public std::__binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const{
            return ((*left) < (*right));
        }
};

/**
 * @brief struct that allows to apply STL methods to containers of strings
 * used in particular by the custom_priority_queue struct and the LimitOrderBook::activeOrders vectors
 *
 * @tparam T
 * Use case is mainly for shared pointers to orders
 */
template<class T>
struct PtrGreater
    : public std::__binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const{
            return ((*left) > (*right));
        }
};



/**
 * @brief A priority queue that inherits from the STL priority_queue
 * It allows to remove elements from the queue
 * It also allows to update the prices of the orders by a certain factor by iterating over the
 * underlying container
 *
 * @tparam T Specifically designed to work with shared pointers to Order objects
 * @tparam S Specifically designed to be used with the PtrLess and PtrGreater structs
 */
template<typename T, typename S>
class custom_priority_queue : public std::priority_queue<T, std::vector<T>, S>
{
private:
    double totalVolume;

public:
    /**
     * @brief Assigns 0 to the total volume
     *
     */
    custom_priority_queue() : totalVolume(0.) { }

    /**
     * @brief returns the total volume of the queue
     *
     * @return double
     */
    double getTotalVolume() const {
        return totalVolume;
    }

    /**
     * @brief pops the top element of the queue and updates the total volume
     *
     * @return double
     */
    void pop() {
        totalVolume -= this -> top() -> volume;
        this -> priority_queue::pop();
    }

    /**
     * @brief updates the volume of the queue and pushes the input parameter into the queue
     *
     * @param value T& the order to be inserted
     */
    void push(const T& value) {
        totalVolume += value -> volume;
        this -> priority_queue::push(value);
    }

    /**
     * @brief iterates through the container and removes the order pointer that matches the input parameter
     *
     * @param value T& the order to be removed
     * @return bool true if the order was removed, false if it was not found
     */
    bool remove(const T& value) {
        auto it = std::find(this -> c.begin(), this -> c.end(), value);

        if (it == this -> c.end()) {
            return false;
        }

        totalVolume -= (*it) -> volume;
        if (it == this -> c.begin()) {
            // deque the top element
            this->pop();
        }
        else {
            // remove element and re-heap
            this->c.erase(it);
            std::make_heap(this->c.begin(), this->c.end(), this->comp);
        }
        return true;
    }

    /**
     * @brief iterates through the container and updates the prices of the orders by the input parameter
     *
     * @param priceChange the factor by which the prices are updated
     */
    void updatePrices(double priceChange) {
        priceChange *= PRICE_CHANGE_FACTOR;
        for (auto orderPtr : this -> c) {
            orderPtr -> addToCurrentPrice(priceChange);
        }
    }

    /**
     * @brief update total volume by the desired amount. Used when updating the orders - say if an order is
     * partially matched.
     *
     * @return double
     */
    void updateTotalVolume(double volumeChange) {
        totalVolume += volumeChange;
    }

    // void updateVolumes(double volumeChange) {
    //     for (auto orderPtr : this -> c) {
    //         orderPtr -> updateOrderVolume(volumeChange);
    //     }
    // }

    /**
     * @brief Prints out the prices and volumes of the orders in the queue
     *
     */
    friend std::ostream & operator<<(std::ostream & os, const custom_priority_queue & pq) {
        for (auto orderPtr : pq.c) {
            os << "Price: " << orderPtr -> price << " Volume: " << orderPtr -> volume << std::endl;
        }
        return os;
    }
};


/***************************** LIMIT ORDER BOOK CLASS ******************************/

/**
 * @brief class that models a limit order book. It has containers for ask prices, bid prices
 * and rules for matching orders.
 *
 */
class LimitOrderBook {
private:
    /**
     * @brief volume weighted adjusted price computed by the matchOrders method
     *
     */
    double vwap;

    /**
     * @brief list of active orders in the book, which comprise unmatched market orders and
     * orders that have just been placed and are waiting to be matched. The list is sorted by time of placement
     *
     */
    std::list<std::shared_ptr<Order>> activeOrders;

    /**
     * @brief min custom priority queue of ask prices
     * contains shared pointers to the orders in the book
     */
    custom_priority_queue<std::shared_ptr<Order>,
                        PtrGreater<std::shared_ptr<Order>>> ask_prices;

    /**
     * @brief max custom priority queue of bid prices
     * contains shared pointers to the orders in the book
     */
    custom_priority_queue<std::shared_ptr<Order>,
                        PtrLess<std::shared_ptr<Order>>> bid_prices;

    // still to implement
    // ExecutionStrategy executionStrategy;

    /**
     * @brief Helper function of the matchOrders method. It takes a shared pointer to the order that needs to be matched
     * and a reference to either ask prices or bid prices, and matches the order with the best prices in the book.
     * Updates the corresponding portfolios in the process.
     * Returns the volume left to match and the volume times the price it was matched at.
     * Currently matches orders partially if either of the traders cannot trade the full volume.
     *
     * @tparam T
     * @param orderPtr order to be matched
     * @param pricePQ priority queue of either buy prices or bid prices
     * @return std::pair<double, double>(): a pair containing the volume that is left to match - used to know if the order needs to be placed -
     *  and amount of wealth that was matched - used to determine the VWAP
     */
    template<typename T>
    std::pair<double, double> walkTheBook(std::shared_ptr<Order> orderPtr, T & pricePQ) {
        assert(orderPtr != nullptr);

        if (pricePQ.empty())
            return {orderPtr -> volume, 0.};

        bool isOrderBuy {orderPtr -> isBuy};
        assert(isOrderBuy != pricePQ.top() -> isBuy);

        double volumeAdjustedOutcome {0.};
        double volumeToMatch {orderPtr -> volume};
        auto currTrader = (orderPtr -> trader).lock();

        if (!currTrader) {
            return {0., 0.};
        }


        bool priceCondition {(!pricePQ.empty()) &&
                             (isOrderBuy ? (pricePQ.top() -> price) <= orderPtr -> price :
                             (pricePQ.top() -> price) >= orderPtr -> price)};

        while (volumeToMatch > 0. && priceCondition) {
            auto bestOrder = pricePQ.top();
            if (auto limitTrader = bestOrder -> trader.lock()) {

                double volumeToMatchWithBestOrder {std::min(volumeToMatch, bestOrder -> volume)};
                bool deleteBestOrder {false};

                // check if the trader currently placing the order can trade the volume
                if (!(currTrader -> canTrade(volumeToMatchWithBestOrder, bestOrder -> price, orderPtr -> isBuy))) {
                    volumeToMatch = 0.; // this exits the while loop and makes sure that matchOrders deletes the order
                    if (orderPtr -> isBuy) {
                        volumeToMatchWithBestOrder = currTrader -> getCash() / bestOrder -> price;
                    } else {
                        volumeToMatchWithBestOrder = currTrader -> getShares();
                    }
                }

                // check if the trader with the order in the book can trade the volume
                if (!(limitTrader -> canTrade(volumeToMatchWithBestOrder, bestOrder -> price, orderPtr -> isBuy))) {
                    deleteBestOrder = true; // this makes sure that the limit order is deleted from the book
                    if (bestOrder -> isBuy) {
                        volumeToMatchWithBestOrder = limitTrader -> getCash() / bestOrder -> price;
                    } else {
                        volumeToMatchWithBestOrder = limitTrader -> getShares();
                    }
                }

                //updating the portfolios. No need to update the fictitious portfolio here
                currTrader -> updateActualPortfolio(volumeToMatchWithBestOrder, bestOrder -> price, orderPtr -> isBuy ? -1. : 1.);
                limitTrader -> updateActualPortfolio(volumeToMatchWithBestOrder, bestOrder -> price, orderPtr -> isBuy ? 1. : -1.);

                // update the amount of volume of the order that still needs to be matched
                volumeToMatch -= volumeToMatchWithBestOrder;

                if (deleteBestOrder || volumeToMatch > 0) { // if the order is not fully matched, delete the best order
                    pricePQ.pop();
                    limitTrader -> deleteActiveOrder(bestOrder);
                } else {
                    pricePQ.updateTotalVolume(-volumeToMatchWithBestOrder);
                    bestOrder -> updateOrderVolume(bestOrder -> volume - volumeToMatchWithBestOrder);
                }

                // add the amount of value matched
                volumeAdjustedOutcome += volumeToMatchWithBestOrder * bestOrder -> price;

                // update price condition
                priceCondition = (!pricePQ.empty()) &&
                                (isOrderBuy ? (pricePQ.top() -> price) <= orderPtr -> price :
                                (pricePQ.top() -> price) >= orderPtr -> price);

                // perhaps should update prices here? For higher precision
            } else {
                pricePQ.pop();
            }
        }

        return std::pair<double, double>(std::max(volumeToMatch, 0.), volumeAdjustedOutcome);
    }


    /**
     * @brief Helper function of matchOrders. It appends the limit order passed to either ask_prices or
     * bid_prices
     *
     */
    void placeLimitOrder(std::shared_ptr<Order>);

    /**
     * @brief Helper function of matchOrders. Currently not used.
     *
     */
    void placeMarketOrder(std::shared_ptr<Order>);

    /**
     * @brief Helper function of matchOrders and also used by MarketPlayer::deleteOrder(). Deletes the limit order
     * from either ask_prices or bid_prices.
     *
     */
    void deleteLimitOrder(std::shared_ptr<Order>);

    /**
     * @brief Helper function of MarketPlayer::deleteOrder(). Deletes an unmatched market order.
     *
     */
    void deleteMarketOrder(std::shared_ptr<Order>);

    /**
     * @brief Helper function of the MatchOrders method. Assigns the input parameter,
     * which is the VWAP of only the current time step, and
     * assigns it to LimitOrderBook::vwap.
     *
     */
    void updateVWAP(double);


    /**
     * @brief Helper function of the matchOrders method. Updates the prices of the active limit orders based on VWAP and
     * book imbalance.
     *
     * @param transaction double: total amount traded
     * @param volume double: total volume traded
     *
     */
    void updateBookPrices();

public:
    /**
     * @brief Construct a new Limit Order Book object
     *
     */
    LimitOrderBook() : vwap(0.) { }

    /**
     * @brief Checks whether the Limit Order book has any active ask limit orders.
     *
     * @return bool
     */
    bool hasAskOrders() const;

    /**
     * @brief Checks whether the Limit Order book has any active bid limit orders.
     *
     * @return bool
     */
    bool hasBidOrders() const;

    /**
     * @brief Get the ask price
     *
     * @return double: the lowest ask price if ask_prices is not empty, LOWER_PRICE_LIMIT otherwise.
     */
    double get_ask() const;

    /**
     * @brief Get the ask price
     *
     * @return double: the lowest ask price if ask_prices is not empty, LOWER_PRICE_LIMIT otherwise.
     */
    double get_bid() const;

    /**
     * @brief Get the mid price of the book.
     *
     * @return double: the average between the ask price and the bid price.
     */
    double get_mid_price() const;

    /**
     * @brief getter for vwap.
     *
     * @return double: the vwap computed only over the last timestep.
     */
    double get_VWAP() const;

    /**
     * @brief Get the volume imbalance of the book. Positive if there are more asks than bids, negative if there
     * are more bids than asks.
     *
     * @return double: the difference between the total ask volume and the total bid volume.
     */
    double getVolumeImbalance() const;

    /**
     * @brief Get the spread between ask and bid. If there are no ask or bid prices, returns 0.
     *
     * @return double: the difference between the ask price and the bid price.
     */
    double getSpread() const;

    /**
     * @brief gets the total volume in the book.
     *
     * @return double
     */
    double get_total_volume() const;

    /**
     * @brief gets the total ask volume in the book.
     *
     * @return double
     */
    double get_total_ask_volume() const;

    /**
     * @brief gets the total bid volume in the book.
     *
     * @return double
     */
    double get_total_bid_volume() const;

    /**
     * @brief
     *
     * @return the number of orders which are currently in either ask_prices, bid_prices or activeOrders
     */
    int getNumActiveOrders() const;

    /**
     * @brief Get the active limit ask orders.
     *
     */
    const custom_priority_queue<std::shared_ptr<Order>,
                                PtrGreater<std::shared_ptr<Order>>> & getAskOrders() const {
        return ask_prices;
    }

    /**
     * @brief Get the current limit bid orders.
     *
     */
    const custom_priority_queue<std::shared_ptr<Order>,
                                PtrLess<std::shared_ptr<Order>>> & getBidOrders() const {
        return bid_prices;
    }

    /**
     * @brief inputs a string into a stream showing the current ask prices, bid prices, and unmatched orders.
     *
     * @return
     */
    friend std::ostream & operator<<(std::ostream &, const LimitOrderBook &);

    /**
     * @brief helper function of the Market::simulateMarket method. Appends an order to the active orders, that will
     * then be matched by the matchOrders method.
     *
     */
    void addOrder(std::shared_ptr<Order>);

    /**
     * @brief deletes the input order on the limit order book by invoking either the deleteLimitOrder or
     * the deleteMarketOrder private methods.
     *
     */
    void deleteOrder(std::shared_ptr<Order>);

    /**
     * @brief Helper function of Market::simulateMarket. Attempts to match the orders currently present in activeOrders
     * with the active limit orders. In the process, it places new limit orders, updates VWAP, updates the traders
     * portfolios, deletes fully matched orders and any orders encountered in the book that traders could not fully
     * match. It also appends any unmatched market orders to the activeOrders list and the active orders of the
     * respective traders.
     *
     *
     */
    void matchOrders();
};
