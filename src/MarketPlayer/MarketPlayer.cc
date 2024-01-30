#include <iostream>
#include <random>
#include <memory>

#include "LimitOrderBook.h"
#include "MarketPlayer.h"


Portfolio::Portfolio(double cash, double stock) : cash{cash}, stock{stock} {}


const double INITIAL_WEALTH{1};
int MarketPlayer::numCurrTraders{0};

MarketPlayer::MarketPlayer() : totalWealth(INITIAL_WEALTH), id(numCurrTraders++), portfolio(0, 0) {
    std::random_device rd;
    auto distribution = std::uniform_real_distribution<double> (-1., 1.);
    portfolio = Portfolio(INITIAL_WEALTH * distribution(rd), INITIAL_WEALTH - portfolio.cash);
    newsResponsiveness = distribution(rd);
    connectionSpeed = std::abs(distribution(rd));
}

double MarketPlayer::getTotalWealth() const {  return totalWealth; }
double MarketPlayer::getCash() const {  return portfolio.cash; }
int MarketPlayer::getId() const { return id; }
double MarketPlayer::getShares() const  { return portfolio.stock; }
std::vector<std::shared_ptr<Order>> MarketPlayer::getActiveOrder() { return activeOrders; }


/*  Deletes an active order of @amountOfShares, if present;
    currently only deletes the shared ptr stored by the @MarketPlayer,
    not the one held by the @LimitOrderBook.
    Returns true if the Market Order has been completely deleted */

void MarketPlayer::deleteActiveOrder(std::shared_ptr<Order> order) {
    // should I instead use remove_if? Cause it may only delete if the address of the pointers is the same
    activeOrders.erase(std::remove(activeOrders.begin(), activeOrders.end(), order), activeOrders.end());
}

void MarketPlayer::deleteActiveOrders() {
    activeOrders.clear();
}

double MarketPlayer::generateTimestamp() const {
    std::random_device rd;
    auto distribution = std::uniform_real_distribution<double> (0., 1.);
    return distribution(rd);
}

double MarketPlayer::computeProbabilityOfTrading(LimitOrderBook & limitOrderBook, double news) const {
    double probabilityOfTrading {1.};
    // if the trader has no active orders, they will always trade
    if (activeOrders.empty())
        return probabilityOfTrading;

    // if the trader has active orders, they will trade with a probability that depends on the news and the spread
    double spread {limitOrderBook.getSpread()};
    double newsImpact {newsResponsiveness * news};
    double spreadImpact {connectionSpeed * spread};
    probabilityOfTrading = 1. / (1. + std::exp(-newsImpact - spreadImpact));
    return probabilityOfTrading;
}

bool MarketPlayer::determineLimitOrMarket(double tradingLikelihood, LimitOrderBook & limitOrderBook) const {
    // if the trader has no active orders, they will always trade
    if (activeOrders.empty())
        return false;

    // if the trader has active orders, they will trade with a probability that depends on the news and the spread
    double spread {limitOrderBook.getSpread()};
    double spreadImpact {connectionSpeed * spread};
    double probabilityOfMarketOrder {1. / (1. + std::exp(-spreadImpact))};
    return tradingLikelihood < probabilityOfMarketOrder;
}

bool MarketPlayer::determineIfBuyOrSell(double tradingLikelihood, LimitOrderBook & limitOrderBook) const {
    // if the trader has no active orders, they will always trade
    if (activeOrders.empty())
        return false;

    // if the trader has active orders, they will trade with a probability that depends on the news and the spread
    double spread {limitOrderBook.getSpread()};
    double spreadImpact {connectionSpeed * spread};
    double probabilityOfBuy {1. / (1. + std::exp(-spreadImpact))};
    return tradingLikelihood < probabilityOfBuy;
}

double MarketPlayer::computePrice(double tradingLikelihood, LimitOrderBook & limitOrderBook) const {
    // if the trader has no active orders, they will always trade
    if (activeOrders.empty())
        return 0.;

    // if the trader has active orders, they will trade with a probability that depends on the news and the spread
    double spread {limitOrderBook.getSpread()};
    double spreadImpact {connectionSpeed * spread};
    double probabilityOfBuy {1. / (1. + std::exp(-spreadImpact))};
    return tradingLikelihood < probabilityOfBuy;
}

double MarketPlayer::computeVolume(double tradingLikelihood, LimitOrderBook & limitOrderBook) const {
    // if the trader has no active orders, they will always trade
    if (activeOrders.empty())
        return 0.;

    // if the trader has active orders, they will trade with a probability that depends on the news and the spread
    double spread {limitOrderBook.getSpread()};
    double spreadImpact {connectionSpeed * spread};
    double probabilityOfBuy {1. / (1. + std::exp(-spreadImpact))};
    return tradingLikelihood < probabilityOfBuy;
}


Order & MarketPlayer::trade(double news, LimitOrderBook & limitOrderBook) {
    double tradingLikelihood = this -> computeProbabilityOfTrading(limitOrderBook, news);

    bool buy {determineIfBuyOrSell(tradingLikelihood, limitOrderBook)};
    double volume {this -> computeVolume(tradingLikelihood, limitOrderBook)};
    double price {this -> computePrice(tradingLikelihood, limitOrderBook)};
    bool isLimit {this -> determineLimitOrMarket(tradingLikelihood, limitOrderBook)};

    //! Should I use std::make_shared<Order> instead and allocate on the heap?
    Order order(price, volume, buy, isLimit, std::make_shared<MarketPlayer>(*this));
    auto orderPtr = std::make_shared<Order>(order);
    return order;
}
