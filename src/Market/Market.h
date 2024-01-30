#pragma once

#include <vector>
#include "LimitOrderBook.h"
#include "MarketPlayer.h"

// random news generator: at each timestep, generate an intensity of the news at that timestep. Rarely has very high values, simulating crashes, etc
struct NewsGenerator{
    double operator();
};

class Market {
private:
    LimitOrderBook limitOrderBook;
    std::vector<MarketPlayer> marketPlayers;
    NewsGenerator newsGenerator;

public:
    explicit Market(unsigned int numTraders);

    // helper functions

    void processOrder(Order &);

    void simulateMarket(unsigned int numSteps);
};
