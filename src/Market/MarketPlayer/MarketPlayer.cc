#include <iostream>
#include <random>
#include <memory>


#include "LimitOrderBook.h"
#include "MarketPlayer.h"


/***************************** PORTFOLIO METHODS *****************************/
Portfolio::Portfolio() : cash{0}, stock{0} {}

Portfolio::Portfolio(double cash, double stock) : cash{cash}, stock{stock} {}

Portfolio Portfolio::operator+(const Portfolio & other) const {
    Portfolio newPortfolio(0, 0);
    newPortfolio.cash = this -> cash + other.cash;
    newPortfolio.stock = this -> stock + other.stock;
    return newPortfolio;
}

Portfolio & Portfolio::operator+=(const Portfolio & other) {
    this -> cash += other.cash;
    this -> stock += other.stock;
    return *this;
}


/***************************** MARKET_PLAYER METHODS *****************************/


int MarketPlayer::numCurrTraders{0};

MarketPlayer::MarketPlayer() : totalWealth(INITIAL_WEALTH) {
    numCurrTraders++;
    std::random_device rd;
    auto distribution = std::uniform_real_distribution<double> (-1., 1.);
    portfolio.cash = INITIAL_WEALTH * std::abs(distribution(rd));
    portfolio.stock = INITIAL_WEALTH - portfolio.cash;
    newsResponsiveness = distribution(rd);
    connectionSpeed = std::abs(distribution(rd));
}

/*********** getters ***********/
double MarketPlayer::getTotalWealth() const {  return totalWealth; }
double MarketPlayer::getCash() const {  return portfolio.cash; }
int MarketPlayer::getId() const { return id; }
double MarketPlayer::getShares() const  { return portfolio.stock; }
std::vector<std::shared_ptr<Order>> MarketPlayer::getActiveOrders() { return activeOrders; }
int MarketPlayer::getNumCurrTraders() const { return numCurrTraders; }

/*********** helper functions ***********/

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

// need to implement this; would it be better to use a struct that doesn't initialize a random device at each call?
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

void MarketPlayer::updatePortfolio(double diffPrice, double diffVolume, double sold) {
    double diffCash {diffPrice * diffVolume * sold};
    diffVolume *= sold * -1.;
    Portfolio diffPortfolio(diffCash, diffVolume);
    portfolio += diffPortfolio;
}

/*********** trading functions ***********/

// perhaps trade should belong to the market, or not a friend of the class
// thing is I don't know how else to get the shared pointer to the order constructor
std::shared_ptr<Order> trade(double news, LimitOrderBook & limitOrderBook, std::shared_ptr<MarketPlayer> traderPtr) {
    double tradingLikelihood = traderPtr -> computeProbabilityOfTrading(limitOrderBook, news);

    bool buy {traderPtr -> determineIfBuyOrSell(tradingLikelihood, limitOrderBook)};
    double volume {traderPtr -> computeVolume(tradingLikelihood, limitOrderBook)};
    double price {traderPtr -> computePrice(tradingLikelihood, limitOrderBook)};
    bool isLimit {traderPtr -> determineLimitOrMarket(tradingLikelihood, limitOrderBook)};

    auto orderPtr = std::make_shared<Order>(price, volume, buy, isLimit, traderPtr);
    return orderPtr;
}
