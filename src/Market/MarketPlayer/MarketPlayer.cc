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
    auto wealthDistribution = std::uniform_real_distribution<double> (0., 1.);
    auto connectionDistribution = std::uniform_real_distribution<double> (0., 1.);
    auto newsDistribution = std::normal_distribution<double> (0., 1.);
    id = numCurrTraders;

    // all traders start with the same amount of total wealth, but different portfolios
    portfolio.cash = INITIAL_WEALTH * wealthDistribution(rd);
    portfolio.stock = INITIAL_WEALTH - portfolio.cash;
    newsResponsiveness = newsDistribution(rd);
    connectionSpeed = connectionDistribution(rd);
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
    return connectionSpeed + distribution(rd);
}

double MarketPlayer::computeProbabilityOfTrading(LimitOrderBook & limitOrderBook, double news) const {
    double probabilityOfTrading {0.5};

    // double spread {limitOrderBook.getSpread()};
    double newsImpact {newsResponsiveness * news};
    // double spreadImpact {connectionSpeed * spread};
    // probabilityOfTrading = 1. / (1. + std::exp(-newsImpact - spreadImpact));
    return probabilityOfTrading * (1. + newsImpact);
}

bool MarketPlayer::determineLimitOrMarket(double tradingLikelihood, LimitOrderBook & limitOrderBook) const {
    return tradingLikelihood < 0.2 || tradingLikelihood > 0.8;
}

bool MarketPlayer::determineIfBuyOrSell(double tradingLikelihood, LimitOrderBook & limitOrderBook) const {
    return tradingLikelihood > 0.5;
}

// Perhaps I should not have tradingLikelihood affect the price as much; it already affect too many things
double MarketPlayer::computePrice(double tradingLikelihood, LimitOrderBook & limitOrderBook, bool isBuy) const {
    double price {0.0};
    if (isBuy) {
        if (limitOrderBook.hasBidOrders()) {
            price = limitOrderBook.get_ask();
        } else {
            price = limitOrderBook.get_mid_price();
        }
    } else {
        if (limitOrderBook.hasAskOrders()) {
            price = limitOrderBook.get_bid();
        } else {
            price = limitOrderBook.get_mid_price();
        }
    }

    if (isBuy) {
        if (tradingLikelihood < 0.2) {
            return price * (1. - tradingLikelihood);
        } else if (tradingLikelihood > 0.8) {
            return price * (1. + tradingLikelihood);
        }
    } else {
        if (tradingLikelihood < 0.2) {
            return price * (1. + tradingLikelihood);
        } else if (tradingLikelihood > 0.8) {
            return price * (1. - tradingLikelihood);
        }
    }
}

double MarketPlayer::computeVolume(double tradingLikelihood, LimitOrderBook & limitOrderBook, bool isBuy, double price) const {
    if (price <= LOWER_PRICE_LIMIT)
        return 0.0;

    double ans {0.};

    if (isBuy)
        ans = (portfolio.cash) * tradingLikelihood;
    else
        ans =  portfolio.stock * tradingLikelihood;

    if (ans < LOWER_VOLUME_LIMIT)
        return 0.0;

    // TODO: I need to check if the trader already has an order that would cause them to go bankrupt

    if (isBuy && price * ans > portfolio.cash)
        return portfolio.cash / price < INITIAL_WEALTH ? portfolio.cash / price : INITIAL_WEALTH;

    return ans < INITIAL_WEALTH ? ans : INITIAL_WEALTH;
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
    double tradingProbability = traderPtr -> computeProbabilityOfTrading(limitOrderBook, news);

    if (tradingProbability < LOWER_TRADING_LIMIT)
        return nullptr;

    bool buy {traderPtr -> determineIfBuyOrSell(tradingProbability, limitOrderBook)};
    double price {traderPtr -> computePrice(tradingProbability, limitOrderBook, buy)};
    if (price <= LOWER_PRICE_LIMIT)
        return nullptr;

    double volume {traderPtr -> computeVolume(tradingProbability, limitOrderBook, buy, price)};
    if (volume <= LOWER_VOLUME_LIMIT)
        return nullptr;

    bool isLimit {traderPtr -> determineLimitOrMarket(tradingProbability, limitOrderBook)};

    auto orderPtr = std::make_shared<Order>(price, volume, buy, isLimit, traderPtr);
    return orderPtr;
}
