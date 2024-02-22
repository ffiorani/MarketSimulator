#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <string>
#include <vector>

#include "Market.h"
#include "MarketPlayer.h"
#include "LimitOrderBook.h"

TEST_CASE("NewsGenerator - how does it affect buy or sell decision? - empty limit order book") {
    auto playerPtr = std::make_shared<MarketPlayer>();
    LimitOrderBook book;
    double news = 0.5;
    int timeStep = 0;

    auto order = trade(news, book, playerPtr);

    REQUIRE(order != nullptr);
    REQUIRE(order -> volume > 0);
    REQUIRE(order -> price > 0);
    REQUIRE(order -> price != Approx(1.0));
}

TEST_CASE("TestMarketCreation", "[Market]") {
    unsigned int numTraders = 10;
    Market market(numTraders);

    REQUIRE(market.getNumTraders() == numTraders);
}


TEST_CASE("TestMarketSimulate", "[Market]") {
    Market market(10);
    std::vector<double> priceHistory;

    market.simulateMarket(100, priceHistory);

    REQUIRE(priceHistory.size() == 100);
}

TEST_CASE("Market simulation") {
    const size_t numTraders {100};
    const size_t numSteps {100};

    Market market(numTraders);

    std::vector<double> priceHistory;
    market.simulateMarket(numSteps, priceHistory);

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
    std::vector<double> priceHistory;
    market.simulateMarket(numSteps, priceHistory);

    // Add additional assertions here to test the behavior of simulateMarket
}

TEST_CASE("Market printMarketData") {
    const size_t numTraders {20};
    const size_t numSteps {10};

    Market market(numTraders);

    std::vector<double> priceHistory;
    market.simulateMarket(numSteps, priceHistory);

    std::string marketData = market.printMarketSummary();

    // Add assertions here to test the market data
}

TEST_CASE("Do traders start trading - one trader scenario") {
    LimitOrderBook limitOrderBook;
    auto playerPtr = std::make_shared<MarketPlayer>();
    double news = 0.5;

    auto orderPtr = trade(news, limitOrderBook, playerPtr);

    REQUIRE(orderPtr != nullptr);
    //REQUIRE(*((orderPtr -> trader).lock()) == *(playerPtr));
    REQUIRE(orderPtr -> volume > 0);
    REQUIRE(orderPtr -> price > 0);
    REQUIRE(orderPtr -> price != Approx(1.0));

    // Add assertions here to test that traders are trading
}
