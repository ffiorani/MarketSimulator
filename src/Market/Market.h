#pragma once

#include <vector>
#include <boost/math/distributions/pareto.hpp>  //this does not give a pareto random number generator
#include "LimitOrderBook.h"
#include "MarketPlayer.h"

// random news generator: at each timestep, generate an intensity of the news at that timestep. Rarely has very high values, simulating crashes, etc
struct NewsGenerator{
    double operator();
}

class Market {
private:
    LimitOrderBook limitOrderBook;
    std::vector<MarketPlayer> marketPlayers;
    NewsGenerator newsGenerator;

public:
    Market(unsigned int numTraders);

    // helper functions

    void simulateMarket(unsigned int numSteps);
