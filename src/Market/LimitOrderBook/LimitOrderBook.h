#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <memory>
#include <vector>
#include <list>
#include <cassert>
#include <iostream>

#include "MarketPlayer.h"


const double UPPER_PRICE_LIMIT = 10000;
const double LOWER_PRICE_LIMIT  = 0.0;
const double INITIAL_PRICE= 1.0;
const double PRICE_CHANGE_FACTOR = 10000.0;

struct Order
{
double price;
double volume;
bool isBuy;
bool isLimit;
std::weak_ptr<MarketPlayer> trader;

Order(double, double, bool, bool, std::shared_ptr<MarketPlayer>);

void updateOrderVolume(double);
void updateOrderPrice(double);

void addToCurrentPrice(double);


bool operator<(const Order &) const;
bool operator>(const Order &) const;

};

template<class T>
struct PtrLess
    : public std::__binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const{
            return ((*left) < (*right));
        }
};

// can I get this from @ptr_less? Should I implement this in the .cc file? how can I do this?
template<class T>
struct PtrGreater
    : public std::__binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const{
            return ((*left) > (*right));
        }
};

template<typename T, typename S>
class custom_priority_queue : public std::priority_queue<T, std::vector<T>, S>
{
public:

    bool remove(const T& value) {
        auto it = std::find(this -> c.begin(), this -> c.end(), value);

        if (it == this -> c.end()) {
            return false;
        }
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

    void updatePrices(double priceChange) {
        priceChange *= PRICE_CHANGE_FACTOR;
        for (auto orderPtr : this -> c) {
            orderPtr -> addToCurrentPrice(priceChange);
        }
    }

    // void updateVolumes(double volumeChange) {
    //     for (auto orderPtr : this -> c) {
    //         orderPtr -> updateOrderVolume(volumeChange);
    //     }
    // }

    friend std::ostream & operator<<(std::ostream & os, const custom_priority_queue & pq) {
        for (auto orderPtr : pq.c) {
            os << "Price: " << orderPtr -> price << " Volume: " << orderPtr -> volume << std::endl;
        }
        return os;
    }
};

class LimitOrderBook {
private:
    double vwap;
    double totalVolume;


    // still to implement
    // ExecutionStrategy executionStrategy;

    // matches the order iteratively with the best prices in the book and returns the volume left and the volume adjusted outcome
    template<typename T>
    std::pair<double, double> walkTheBook(std::shared_ptr<Order> orderPtr, T & book) {
        if (book.empty())
            return {orderPtr -> volume, 0.};

        assert(orderPtr -> isBuy != book.top() -> isBuy);

        double volumeAdjustedOutcome {0.};
        double volumeToMatch {orderPtr -> volume};
        auto currTrader = (orderPtr -> trader).lock();


        while (volumeToMatch > 0. && !book.empty() && (book.top() -> price) <= orderPtr -> price) {
            auto bestOrder = book.top();
            auto limitTrader = bestOrder -> trader.lock();
            double volumeToMatchWithBestOrder {std::min(volumeToMatch, bestOrder -> volume)};

            // update the trader's portfolio
            currTrader -> updatePortfolio(volumeToMatchWithBestOrder, bestOrder -> price, orderPtr -> isBuy ? -1. : 1.);
            limitTrader -> updatePortfolio(volumeToMatchWithBestOrder, bestOrder -> price, orderPtr -> isBuy ? 1. : -1.);

            volumeToMatch -= volumeToMatchWithBestOrder;
            if (volumeToMatch > 0) { // if the order is not fully matched, delete the best order
                book.pop();
                limitTrader -> deleteActiveOrder(bestOrder);
            } else {
                bestOrder -> updateOrderVolume(volumeToMatchWithBestOrder * -1.);
            }

            volumeAdjustedOutcome += volumeToMatchWithBestOrder * bestOrder -> price;

            // update the total volume
            totalVolume -= volumeToMatchWithBestOrder;
        }

        return std::pair<double, double>(std::max(volumeToMatch, 0.), volumeAdjustedOutcome);
    }

    void placeLimitOrder(std::shared_ptr<Order>);
    void deleteLimitOrder(std::shared_ptr<Order>);
    void appendMarketOrder(std::shared_ptr<Order>);
    // currently computes vwap day by day
    void updateVWAP(double);

public:
// make this private once done with testing
/* The following linked list contains the Market orders that have not been matched
       and the current orders being placed */
    std::list<std::shared_ptr<Order>> activeOrders;
    // make these private once done with testing
    custom_priority_queue<std::shared_ptr<Order>,
                        PtrGreater<std::shared_ptr<Order>>> ask_prices;

    custom_priority_queue<std::shared_ptr<Order>,
                        PtrLess<std::shared_ptr<Order>>> bid_prices;

    friend std::ostream & operator<<(std::ostream &, const LimitOrderBook &);
    double get_ask() const;
    double get_bid() const;
    double get_mid_price() const;
    double get_VWAP() const;
    double getSpread() const;
    double get_total_volume() const;
    int getNumActiveOrders() const;
    void updateBookPrices(double, double);


    void addOrder(std::shared_ptr<Order>);

    /**
     * @brief
     * the method is invoked every time a new order is added to the book
     * takes the last order added as input, and matches it with the lowest bid/ ask prices already available
     */
    void matchOrders();
};
