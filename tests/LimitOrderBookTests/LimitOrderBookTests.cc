#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <memory>
#include "LimitOrderBook.h"
#include "MarketPlayer.h"

TEST_CASE("Limit Order Book Tests") {
    SECTION("Add Order") {
        LimitOrderBook book;
        auto trader = std::make_shared<MarketPlayer>();
        auto order = std::make_shared<Order>(150.0, 100, false, true, trader);
        book.addOrder(order);
        REQUIRE(book.getNumActiveOrders() == 1);
    }

    SECTION("Match Orders - one order") {
        LimitOrderBook book;
        auto trader = std::make_shared<MarketPlayer>();
        auto order = std::make_shared<Order>(150.0, 100, false, true, trader);
        book.addOrder(order);
        REQUIRE(book.getNumActiveOrders() == 1);
        REQUIRE(book.activeOrders.size() == 1);
        book.matchOrders();
        REQUIRE(book.activeOrders.size() == 0);
        REQUIRE(book.getNumActiveOrders() == 1);
        REQUIRE(trader->getCash() <= INITIAL_WEALTH);
        REQUIRE(trader->getShares() >= 0);
        REQUIRE(trader->getShares() <= INITIAL_WEALTH);
        REQUIRE(trader->getCash() >= 0);
        REQUIRE(trader->getTotalWealth() == INITIAL_WEALTH);
        REQUIRE(book.ask_prices.size() == 1);
        REQUIRE(book.bid_prices.size() == 0);
    }

    SECTION("Match Orders - two orders and two traders") {
        LimitOrderBook book;
        auto trader1 = std::make_shared<MarketPlayer>();
        auto trader2 = std::make_shared<MarketPlayer>();
        auto order1 = std::make_shared<Order>(10.0, 100, false, true, trader1);
        auto order2 = std::make_shared<Order>(20.0, 150, true, true, trader2);
        book.addOrder(order1);
        book.addOrder(order2);
        book.matchOrders();
        // this is before not allowing negative cash and stock
        // need to take into account update of price
        REQUIRE(trader2->getCash() <= Approx(-999.0));
        REQUIRE(trader2->getCash() >= Approx(-1001.0));
        REQUIRE(trader1->getCash() <= Approx(1001.0));
        REQUIRE(trader1->getCash() >= Approx(999.0));
        REQUIRE(trader2->getShares() == 100);
        REQUIRE(trader1->getShares() == -100);
        REQUIRE(book.ask_prices.size() == 0);
        REQUIRE(book.bid_prices.size() == 1);
        REQUIRE(book.get_bid() >= Approx(19.0));
        REQUIRE(book.get_bid() <= Approx(21.0));
        REQUIRE(trader2->getActiveOrders().size() == 1);
        REQUIRE(trader2->getActiveOrders().at(0) -> volume == 50);
        REQUIRE(trader1->getActiveOrders().size() == 0);

    }

    SECTION("Get Bid") {
        LimitOrderBook book;
        auto trader = std::make_shared<MarketPlayer>();
        auto order1 = std::make_shared<Order>(150.0, 100, true, true, trader);
        auto order2 = std::make_shared<Order>(155.0, 100, true, true, trader);
        book.addOrder(order1);
        book.addOrder(order2);
        book.matchOrders();
        // need to take into account update of price
        REQUIRE(book.get_bid() >= 155.0);
    }

    SECTION("Get Ask") {
        LimitOrderBook book;
        auto trader1 = std::make_shared<MarketPlayer>();
        auto trader2 = std::make_shared<MarketPlayer>();
        auto order1 = std::make_shared<Order>(150.0, 100, false, true, trader1);
        auto order2 = std::make_shared<Order>(155.0, 100, false, true, trader2);
        book.addOrder(order1);
        book.addOrder(order2);
        book.matchOrders();
        // need to take into account update of price
        REQUIRE(book.get_ask() <= 150.0);
    }

    SECTION("Update Book Prices") {
        LimitOrderBook book;
        auto trader1 = std::make_shared<MarketPlayer>();
        auto trader2 = std::make_shared<MarketPlayer>();
        auto orderPtr1 = std::make_shared<Order>(20.0, 100, false, true, trader1);
        auto orderPtr2 = std::make_shared<Order>(10.0, 100, true, true, trader2);
        book.addOrder(orderPtr1);
        book.addOrder(orderPtr2);
        REQUIRE(book.activeOrders.size() == 2);
        book.matchOrders();
        REQUIRE(book.activeOrders.size() == 0);
        //REQUIRE(trader2->getCash() == 15000.0);
        REQUIRE(book.ask_prices.size() == 1);
        REQUIRE(book.bid_prices.size() == 1);
        REQUIRE(book.get_bid() == 10.0);
        REQUIRE(book.get_ask() == 20.0);
        REQUIRE(book.get_mid_price() == 15.0);
        REQUIRE(book.getSpread() == 10.0);
        REQUIRE(book.get_total_volume() == 200.0);

        double prev_bid = book.get_bid();
        double prev_ask = book.get_ask();
        book.updateBookPrices(1200.0, 100.0);
        REQUIRE(book.get_bid() == Approx(9.97));
        REQUIRE(book.get_ask() == Approx(19.97));
    }
}
