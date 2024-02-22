#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <memory>
#include "LimitOrderBook.h"
#include "MarketPlayer.h"

TEST_CASE("Custom Priority Queue Tests") {
    SECTION("TestPushAndTop", "[custom_priority_queue]") {
        custom_priority_queue<std::shared_ptr<Order>, PtrLess<std::shared_ptr<Order>>> pq;

        auto order1 = std::make_shared<Order>(1, 10, true, true, nullptr);
        auto order2 = std::make_shared<Order>(2, 20, true, true, nullptr);
        auto order3 = std::make_shared<Order>(3, 30, true, true, nullptr);

        pq.push(order1);
        pq.push(order2);
        pq.push(order3);

        REQUIRE(pq.top()->price == 3);
    }

    SECTION("TestPop", "[custom_priority_queue]") {
        custom_priority_queue<std::shared_ptr<Order>, PtrLess<std::shared_ptr<Order>>> pq;

        auto order1 = std::make_shared<Order>(1, 10, true, true, nullptr);
        auto order2 = std::make_shared<Order>(2, 20, true, true, nullptr);
        auto order3 = std::make_shared<Order>(3, 30, true, true, nullptr);

        pq.push(order1);
        pq.push(order2);
        pq.push(order3);

        pq.pop();
        REQUIRE(pq.top()->price == 2);

        pq.pop();
        REQUIRE(pq.top()->price == 1);
    }

    SECTION("TestEmpty", "[custom_priority_queue]") {
        custom_priority_queue<std::shared_ptr<Order>, PtrLess<std::shared_ptr<Order>>> pq;

        REQUIRE(pq.empty() == true);

        auto order1 = std::make_shared<Order>(1, 10, true, true, nullptr);

        pq.push(order1);
        REQUIRE(pq.empty() == false);

        pq.pop();
        REQUIRE(pq.empty() == true);
    }

    SECTION("TestTotalVolume", "[custom_priority_queue]") {
        custom_priority_queue<std::shared_ptr<Order>, PtrLess<std::shared_ptr<Order>>> pq;

        auto order1 = std::make_shared<Order>(1, 10, true, true, nullptr);
        auto order2 = std::make_shared<Order>(2, 20, true, true, nullptr);
        auto order3 = std::make_shared<Order>(3, 30, true, true, nullptr);

        pq.push(order1);
        pq.push(order2);
        pq.push(order3);

        REQUIRE(pq.getTotalVolume() == 60);

        pq.pop();
        REQUIRE(pq.getTotalVolume() == 30);

        pq.updateTotalVolume(-10);
        REQUIRE(pq.getTotalVolume() == 20);
    }
    // Add more test cases for other methods...
}

TEST_CASE("Limit Order Book Tests") {
    SECTION("Test hasAskOrders()") {
        LimitOrderBook book;
        REQUIRE(book.hasAskOrders() == false);

        std::shared_ptr<Order> askOrder = std::make_shared<Order>(10.0, 100.0, false, true, nullptr);
        book.addOrder(askOrder);
        book.matchOrders();
        REQUIRE(book.hasAskOrders() == true);
    }

    SECTION("Test hasBidOrders()") {
        LimitOrderBook book;
        REQUIRE(book.hasBidOrders() == false);

        std::shared_ptr<Order> bidOrder = std::make_shared<Order>(10.0, 100.0, true, true, nullptr);
        book.addOrder(bidOrder);
        book.matchOrders();
        REQUIRE(book.hasBidOrders() == true);
    }

    SECTION("Test get_ask()") {
        LimitOrderBook book;
        REQUIRE(book.get_ask() == UPPER_PRICE_LIMIT);

        std::shared_ptr<Order> askOrder = std::make_shared<Order>(10.0, 100.0, false, true, nullptr);
        book.addOrder(askOrder);
        book.matchOrders();
        REQUIRE(book.get_ask() == 10.0);
    }

    SECTION("Test get_bid()") {
        LimitOrderBook book;
        REQUIRE(book.get_bid() == LOWER_PRICE_LIMIT);

        std::shared_ptr<Order> bidOrder = std::make_shared<Order>(10.0, 100.0, true, true, nullptr);
        book.addOrder(bidOrder);
        book.matchOrders();
        REQUIRE(book.get_bid() == 10.0);
    }

    // Add more test cases for other methods...


}
