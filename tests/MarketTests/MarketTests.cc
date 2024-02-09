#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <string>

#include "Market.h"

TEST_CASE("Market simulation") {
    const size_t numTraders {100};
    const size_t numSteps {100};

    Market market(numTraders);

    market.simulateMarket(numSteps);

    REQUIRE(market.printMarketSummary() != "1");
}

TEST_CASE("Market constructor") {
    const size_t numTraders {50};

    Market market(numTraders);

    REQUIRE(market.getNumTraders() == numTraders);
}

TEST_CASE("Market simulateMarket") {
    const size_t numTraders {10};
    const size_t numSteps {5};

    Market market(numTraders);

    market.simulateMarket(numSteps);

    // Add additional assertions here to test the behavior of simulateMarket
}

TEST_CASE("Market printMarketData") {
    const size_t numTraders {20};
    const size_t numSteps {10};

    Market market(numTraders);

    market.simulateMarket(numSteps);

    std::string marketData = market.printMarketSummary();

    // Add assertions here to test the market data
}
