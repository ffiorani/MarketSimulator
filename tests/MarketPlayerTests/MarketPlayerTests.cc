#define CATCH_CONFIG_MAIN


#include <catch2/catch.hpp>
#include "MarketPlayer.h"
#include "LimitOrderBook.h"

TEST_CASE("MarketPlayer initialization") {
    auto playerPtr = std::make_shared<MarketPlayer>();

    SECTION("Total wealth should be equal to initial wealth") {
        REQUIRE(playerPtr -> getTotalWealth() == INITIAL_WEALTH);
    }

    SECTION("Cash and stock should be initialized correctly") {
        REQUIRE(playerPtr -> getCash() >= 0);
        REQUIRE(playerPtr -> getShares() >= 0);
        REQUIRE(playerPtr -> getCash() + playerPtr -> getShares() == INITIAL_WEALTH);
    }

    SECTION("Number of current traders should be incremented") {
        REQUIRE(playerPtr -> getNumCurrTraders() == 1);
    }
}

TEST_CASE("MarketPlayer trading functions") {
    auto playerPtr = std::make_shared<MarketPlayer>();
    LimitOrderBook limitOrderBook;

    SECTION("Probability of trading should be between 0 and 1") {
        double news = 0.5;
        double probability = playerPtr -> computeProbabilityOfTrading(limitOrderBook, news);
        REQUIRE(probability >= 0.0);
        REQUIRE(probability <= 1.0);
    }

    SECTION("Probability of trading should be between 0 and 1 - negative news") {
        double news = -0.5;
        double probability = playerPtr -> computeProbabilityOfTrading(limitOrderBook, news);
        REQUIRE(probability >= 0.0);
        REQUIRE(probability <= 1.0);
    }

    SECTION("Determining limit or market order should return correct result") {
        double tradingLikelihood = 0.5;
        bool isLimit = playerPtr -> determineLimitOrMarket(tradingLikelihood, limitOrderBook);
        REQUIRE(isLimit == false);
    }

    SECTION("Determining if buy or sell should return correct result") {
        double tradingLikelihood = 0.5;
        bool isBuy = playerPtr -> determineIfBuyOrSell(tradingLikelihood, limitOrderBook);
        REQUIRE(isBuy == false);
    }

    SECTION("Computing price should return correct result") {
        double tradingLikelihood = 0.5;
        double price = playerPtr -> computePrice(tradingLikelihood, limitOrderBook, true);
        REQUIRE(price == 0.5);
    }

    SECTION("Computing price should return correct result - sell order") {
        double tradingLikelihood = 0.5;
        double price = playerPtr -> computePrice(tradingLikelihood, limitOrderBook, false);
        REQUIRE(price == 0.5);
    }

    SECTION("Computing volume should return correct result") {
        double tradingLikelihood = 0.5;
        double price = playerPtr -> computePrice(tradingLikelihood, limitOrderBook, true);
        double volume = playerPtr -> computeVolume(tradingLikelihood, limitOrderBook, true, price);
        REQUIRE(volume == 0.5);
    }

    SECTION("Computing volume should return correct result - sell order") {
        double tradingLikelihood = 0.5;
        double price = playerPtr -> computePrice(tradingLikelihood, limitOrderBook, true);
        double volume = playerPtr -> computeVolume(tradingLikelihood, limitOrderBook, false, price);
        REQUIRE(volume == 0.5);
    }

    SECTION("Updating portfolio should modify cash and stock correctly") {
        double diffPrice = 10.0;
        double diffVolume = 2.0;
        double sold = 0.5;

        playerPtr -> updatePortfolio(diffPrice, diffVolume, sold);

        REQUIRE(playerPtr -> getCash() == INITIAL_WEALTH - (diffPrice * diffVolume * sold));
        REQUIRE(playerPtr -> getShares() == diffVolume * sold * -1.0);
    }

    SECTION("Getting id should return a valid id") {
        int id = playerPtr -> getId();
        REQUIRE(id >= 0);
    }

    SECTION("Compute Volume should return a valid volume") {
        double tradingLikelihood = 0.5;
        double price = playerPtr -> computePrice(tradingLikelihood, limitOrderBook, true);
        double volume = playerPtr -> computeVolume(tradingLikelihood, limitOrderBook, true, price);
        REQUIRE(volume > 0.01);
    }

    SECTION("Compute Volume should return a valid volume - sell order") {
        double tradingLikelihood = 0.5;
        double price = playerPtr -> computePrice(tradingLikelihood, limitOrderBook, true);
        double volume = playerPtr -> computeVolume(tradingLikelihood, limitOrderBook, false, price);
        REQUIRE(volume > 0.01);
    }

    SECTION("Deleting active order should remove the order") {
        auto orderPtr = std::make_shared<Order>(150.0, 100, false, true, playerPtr);
        playerPtr -> deleteActiveOrder(orderPtr);
        REQUIRE(playerPtr -> getActiveOrders().size() == 0);
    }

    SECTION("Generating timestamp should return a valid timestamp") {
        double timestamp = playerPtr -> generateTimestamp();
        REQUIRE(timestamp >= 0.0);
    }
    SECTION("Getting active orders should return a vector of orders") {
        auto orderPtr = std::make_shared<Order>(150.0, 100, false, true, playerPtr);
        limitOrderBook.addOrder(orderPtr);
        REQUIRE(playerPtr -> getActiveOrders().size() == 1);
    }

    SECTION("Deleting active orders should remove all orders") {
        auto orderPtr = std::make_shared<Order>(150.0, 100, false, true, playerPtr);
        limitOrderBook.addOrder(orderPtr);
        playerPtr -> deleteActiveOrders();
        REQUIRE(playerPtr -> getActiveOrders().size() == 0);
    }

    SECTION("Trading should return a valid order") {
        double news = 0.5;
        std::shared_ptr<Order> orderPtr = trade(news, limitOrderBook, playerPtr);

        REQUIRE(orderPtr != nullptr);
        REQUIRE(orderPtr->price >= 0.0);
        REQUIRE(orderPtr->volume >= 0.0);
    }
}
