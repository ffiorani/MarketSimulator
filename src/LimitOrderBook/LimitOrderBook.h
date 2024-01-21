#pragma once

#include <vector>
#include <queue>
#include <functional>
#include "ExecutionStrategy.h"

// the first entry of the LimitOrder is the price, the second the volume
typedef std::pair<double, int> LimitOrder;

template<class T>
struct ptrLess
    : public std::binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const{
            return ((*left) <( *right));
        }
};

// can I get this from @ptr_less? Should I implement this in the .cc file? how can I do this?
template<class T>
struct ptrGreater
    : public std::binary_function<T, T, bool> {
        bool operator()(const T& left, const T& right) const{
            return ((*left) >( *right));
        }
};

class LimitOrderBook {
private:
    std::priority_queue<std::shared_ptr<LimitOrder>,
                        std::vector<std::shared_ptr<LimitOrder>>,
                        ptrGreater<std::shared_ptr<LimitOrder>>> ask_prices;

    std::priority_queue<std::shared_ptr<LimitOrder>,
                        std::vector<std::shared_ptr<LimitOrder>>,
                        ptrLess<std::shared_ptr<LimitOrder>>> bid_prices;

    ExecutionStrategy executionStrategy;

public:
    double get_ask() const;
    double get_bid() const;
    double get_mid_price() const;
    double get_VWAP() const;

    void placeLimitOrder(LimitOrder limitOrder);
    void cancelLimitOrder(LimitOrder limitOrder);
    void updatePrice();

};
