#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <memory>
#include <vector>
#include <list>

#include "ExecutionStrategy.h"
#include "MarketPlayer.h"


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


bool operator<(const Order &);

};

template<class T>
struct PtrLess
    : public std::__binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const{
            return ((*left) <( *right));
        }
};

// can I get this from @ptr_less? Should I implement this in the .cc file? how can I do this?
template<class T>
struct PtrGreater
    : public std::__binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const{
            return ((*left) >( *right));
        }
};

template<typename T, typename S>
class custom_priority_queue : public std::priority_queue<T, std::vector<T>, S>
{
public:

    bool remove(const T& value) {
        auto it = std::find(this->c.begin(), this->c.end(), value);

        if (it == this->c.end()) {
            return false;
        }
        if (it == this->c.begin()) {
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

    void updatePrices(double volumeAdjustedOutcome) {
        for (auto& order : this -> c) {
            order -> addToCurrentPrice(volumeAdjustedOutcome);
        }
    }
};

static constexpr double UPPER_PRICE_LIMIT {10000.};
static constexpr double LOWER_PRICE_LIMIT {0.};
static constexpr double INITIAL_PRICE {1.};

class LimitOrderBook {
private:
    double vwap;
    double totalVolume;

    custom_priority_queue<std::shared_ptr<Order>,
                        PtrGreater<std::shared_ptr<Order>>> ask_prices;

    custom_priority_queue<std::shared_ptr<Order>,
                        PtrLess<std::shared_ptr<Order>>> bid_prices;

    /* The following linked list contains the Market orders that have not been matched
       and the current orders being placed */
    std::list<std::shared_ptr<Order>> activeOrders;


    // still to implement
    ExecutionStrategy executionStrategy;

public:
    double get_ask() const;
    double get_bid() const;
    double get_mid_price() const;
    double get_VWAP() const;
    double getSpread() const;
    double get_total_volume() const;

    void placeLimitOrder(std::shared_ptr<Order>);
    void deleteLimitOrder(std::shared_ptr<Order>);
    void appendMarketOrder(std::shared_ptr<Order>);

    /* the method is invoked every time a new order is added to the book
       takes the last order added as input, and matches it with the lowest bid/ ask prices already available */
    void matchOrders();

    /* @matchOrders keeps track of all the transactions occurring at a specific time
       and their respective volumes. The price of each limit order is shifted by the same quantity:
       if buy order, increase volume, else decrease it */
    void updatePrice(double);

    // currently computes vwap day by day
    void updateVWAP(double);


};
