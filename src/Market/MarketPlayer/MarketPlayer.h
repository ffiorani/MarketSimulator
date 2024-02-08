#pragma once

#include <memory>
#include <vector>


const double INITIAL_WEALTH = 1.0;

class Order;
class LimitOrderBook;

// first term is the cash wealth, the second the number of shares
struct Portfolio {
    double cash;
    double stock;
    Portfolio();
    Portfolio(double, double);
    Portfolio operator+(const Portfolio &) const;
    Portfolio operator+=(const Portfolio &);
};

class MarketPlayer {
    private:
        static int numCurrTraders;
        int id;
        Portfolio portfolio;
        double totalWealth;
        double connectionSpeed;
        double newsResponsiveness;

        /* not storing this as a priority queue cause I am expecting to be adding more often for large n
         than deleting - which requires sorting as I am adopting the policy to delete
         the orders further away from the spread when placing a market order of the same kind
         UPDATE: perhaps I do not need the MarketPlayer to know about his/her active orders, after all
         I only need to update the portfolio once the orders are matched, so I can have an order
         point to the market player. However, the MarketPlayer may want to know about their active orders
         in case it affects their strategy: we may not want to place too many orders at the same time to avoid
         going bankrupt*/
        std::vector<std::shared_ptr<Order>> activeOrders;

    public:
        MarketPlayer();
        ~MarketPlayer() { numCurrTraders--; }

        /************* getters *************/

        double getTotalWealth() const;
        double getCash() const;
        int getId() const;
        double getShares() const;
        std::vector<std::shared_ptr<Order>> getActiveOrders();

        /************* helper functions *************/
        double generateTimestamp() const;

        double computeProbabilityOfTrading(LimitOrderBook &, double) const;
        // currently not allowing traders to change or partially delete active orders, without placing an opposite limit order or a market order
        // bool determineLimitOrMarket(double, LimitOrderBook &);
        void deleteActiveOrders();
        void deleteActiveOrder(std::shared_ptr<Order>);

        double computePrice(double, LimitOrderBook &) const;
        double computeVolume(double, LimitOrderBook &) const;

        bool determineLimitOrMarket(double, LimitOrderBook &) const;
        bool determineIfBuyOrSell(double, LimitOrderBook &) const;

        void updatePortfolio(double, double, double);

        /************* trading functions *************/

        // returns the order the trader wants to request
        std::shared_ptr<Order> trade(double, LimitOrderBook &);

};
