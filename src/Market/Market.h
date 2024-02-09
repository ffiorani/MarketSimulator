#pragma once

#include <vector>
#include <random>
#include <iostream>
#include <string>

#include "LimitOrderBook.h"
#include "MarketPlayer.h"

// random news generator: at each timestep, generate an intensity of the news at that timestep. Rarely has very high values, simulating crashes, etc
struct NewsGenerator {
    std::random_device rd;
    std::mt19937 gen;
    std::normal_distribution<> dis;

    NewsGenerator() : gen(rd()), dis(0, 1) {} // mean 0, standard deviation 1

    double operator()() {
        return dis(gen);
    }
};

class Market {
private:
    LimitOrderBook limitOrderBook;
    std::vector<std::shared_ptr<MarketPlayer>> marketPlayers;
    NewsGenerator newsGenerator;

public:
    explicit Market(unsigned int numTraders);

    // getters

    unsigned int getNumTraders() const;
    // helper functions
    std::string printMarketSummary() const;

    /**
     * @brief
     * Runs the market simulation for @numSteps timesteps
     * @param numSteps
     */
    void simulateMarket(unsigned int numSteps);

    friend std::ostream & operator<<(std::ostream & os, const Market & market);
};
