#define CATCH_CONFIG_MAIN


#include <catch2/catch.hpp>
#include "MarketPlayer.h"
#include "LimitOrderBook.h"


TEST_CASE("TestMarketPlayerCreation", "[MarketPlayer]") {
    MarketPlayer player;

    REQUIRE(player.getCash() >= 0);
    REQUIRE(player.getShares() >= 0);
    REQUIRE(player.getId() >= 0);
    REQUIRE(player.getNumCurrTraders() >= 1);
}

TEST_CASE("TestMarketPlayerTrade", "[MarketPlayer]") {
    auto playerPtr = std::make_shared<MarketPlayer>();
    LimitOrderBook book;
    double news = 0.0;
    int timeStep = 0;

    auto order = trade(news, book, playerPtr);

    // Assuming trade function returns nullptr when the order is not valid
    REQUIRE(order == nullptr);
}

TEST_CASE("TestMarketPlayerAddOrder", "[MarketPlayer]") {
    auto playerPtr = std::make_shared<MarketPlayer>();
    auto order = std::make_shared<Order>(1, 10, true, true, playerPtr);

    playerPtr -> addOrder(order);

    // Assuming the order is a buy order
    auto buyOrders = playerPtr -> getBuyOrders();
    REQUIRE(std::find(buyOrders.begin(), buyOrders.end(), order) != buyOrders.end());
}

TEST_CASE("TestMarketPlayerUpdateActualPortfolio", "[MarketPlayer]") {
    MarketPlayer player;
    double initialCash = player.getCash();
    double initialShares = player.getShares();

    player.updateActualPortfolio(10, 1, false); // Assuming this is a buy order

    REQUIRE(player.getCash() == initialCash - 10);
    REQUIRE(player.getShares() == initialShares + 1);
}

TEST_CASE("TestMarketPlayerRemoveOrder", "[MarketPlayer]") {
    auto playerPtr = std::make_shared<MarketPlayer>();
    auto order = std::make_shared<Order>(1, 10, true, true, playerPtr);

    playerPtr -> addOrder(order);
    playerPtr -> deleteActiveOrder(order);

    // Assuming the order is a buy order
    auto buyOrders = playerPtr -> getBuyOrders();
    REQUIRE(std::find(buyOrders.begin(), buyOrders.end(), order) == buyOrders.end());
}

TEST_CASE("TestMarketPlayerUpdatePotentialPortfolio", "[MarketPlayer]") {
    MarketPlayer player;
    double initialPotentialCash = player.getCashIfOrdersMatched();
    double initialPotentialShares = player.getSharesIfOrdersMatched();

    player.updatePortfolioIfOrdersMatched(10, 1, false); // Assuming this is a buy order

    REQUIRE(player.getCashIfOrdersMatched() == initialPotentialCash - 10);
    REQUIRE(player.getSharesIfOrdersMatched() == initialPotentialShares + 1);
}

TEST_CASE("TestMarketPlayerResetPotentialPortfolio", "[MarketPlayer]") {
    MarketPlayer player;

    player.updatePortfolioIfOrdersMatched(10, 1, false); // Assuming this is a buy order

    REQUIRE(player.getCashIfOrdersMatched() == player.getCash());
    REQUIRE(player.getSharesIfOrdersMatched() == player.getShares());
}
