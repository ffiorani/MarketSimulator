#pragma once

#include <memory>
#include "LimitOrderBook.h"

// first term is the cash wealth, the second the number of shares
typedef std::pair<double, double> Portfolio;

class MarketPlayer {
    private:
        static int numCurrTraders = 0;
        int id;
        Portfolio portfolio;
        double totalWealth;
        double connectionSpeed;
        double newsResponsiveness;
        std::shared_ptr<LimitOrder> activeOrder;

    public:
        MarketPlayer();

        double getTotalWealth() const;
        double getCash() const;
        int getId() const;
        double getShares() const;
        std::shared_ptr<LimitOrder> getActiveOrder();

        double computeProbabilityOfTrading(LimitOrderBook &, double) const;
        void placeLimitOrder(double, LimitOrderBook &);
        void placeMarketOrder(double, LimitOrderBook &);
        bool deleteActiveOrder(double);
        bool determineIfMarketOrder(double);


        void trade(LimitOrderBook &, double);
};
