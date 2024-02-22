#include <iostream>
#include <random>
#include <memory>
#include <math.h>


#include "LimitOrderBook.h"
#include "MarketPlayer.h"


/***************************** PORTFOLIO METHODS *****************************/
Portfolio::Portfolio() : cash{0}, stock{0} {}

Portfolio::Portfolio(double cash, double stock) : cash{cash}, stock{stock} {}

Portfolio::Portfolio(const Portfolio & other) : cash{other.cash}, stock{other.stock} {}

Portfolio & Portfolio::operator=(const Portfolio & other) {
    if (this != &other) {
        cash = other.cash;
        stock = other.stock;
    }
    return *this;
}

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



MarketPlayer::MarketPlayer() {
    numCurrTraders++;
    std::random_device rd;
    auto wealthDistribution = std::uniform_real_distribution<double> (0., 1.);
    auto connectionDistribution = std::uniform_real_distribution<double> (0., 1.);
    auto newsDistribution = std::normal_distribution<double> (0., 1.);
    auto riskDistribution = std::uniform_real_distribution<double> (-0.5, 0.5);
    id = numCurrTraders;

    // all traders start with the same amount of total wealth, but different portfolios
    actualPortfolio.cash = INITIAL_WEALTH * wealthDistribution(rd);
    actualPortfolio.stock = INITIAL_WEALTH - actualPortfolio.cash;
    portfolioIfOrdersMatched = actualPortfolio;
    newsResponsiveness = newsDistribution(rd);
    riskAversion = riskDistribution(rd);
    connectionSpeed = connectionDistribution(rd);
}

/*********** getters ***********/

double MarketPlayer::getTotalWealth(const LimitOrderBook & book) const {
    return actualPortfolio.cash + actualPortfolio.stock * book.get_mid_price();
}

double MarketPlayer::getWealthIfOrdersMatched(const LimitOrderBook & book) const {
    return portfolioIfOrdersMatched.cash + portfolioIfOrdersMatched.stock * book.get_mid_price();
}

double MarketPlayer::getCash() const {  return actualPortfolio.cash; }
double MarketPlayer::getCashIfOrdersMatched() const { return portfolioIfOrdersMatched.cash; }
int MarketPlayer::getId() const { return id; }
double MarketPlayer::getShares() const  { return actualPortfolio.stock; }
double MarketPlayer::getSharesIfOrdersMatched() const { return portfolioIfOrdersMatched.stock; }
std::list<std::shared_ptr<Order>> const & MarketPlayer::getBuyOrders() const { return buyOrders; }
std::list<std::shared_ptr<Order>> const & MarketPlayer::getSellOrders() const { return sellOrders; }
int MarketPlayer::getNumCurrTraders() const { return numCurrTraders; }

/*********** delete order methods ***********/

/*  Deletes an active order of @amountOfShares, if present;
    currently only deletes the shared ptr stored by the @MarketPlayer,
    not the one held by the @LimitOrderBook.
    Returns true if the Market Order has been completely deleted */

bool MarketPlayer::deleteOldestOrders(double volume, LimitOrderBook & limitOrderBook, bool isBuy) {

    std::list<std::shared_ptr<Order>>& orders = isBuy ? buyOrders : sellOrders;

    while (volume > 0 && !orders.empty()) {
        auto orderPtr = orders.front();
        if (orderPtr -> volume <= volume) {
            updatePortfolioIfOrdersMatched(orderPtr);
            limitOrderBook.deleteOrder(orderPtr);
            orders.pop_front();
            volume -= orderPtr -> volume;
        } else {
            updatePortfolioIfOrdersMatched(orderPtr -> price, volume, isBuy);
            orderPtr -> volume -= volume;
            volume = 0;
        }
    }
    return volume == 0;
}

void MarketPlayer::deleteActiveOrder(std::shared_ptr<Order> orderPtr, LimitOrderBook & limitOrderBook) {
    bool isBuy {orderPtr -> isBuy};
    std::list<std::shared_ptr<Order>> & orders = isBuy ? buyOrders : sellOrders;
    updatePortfolioIfOrdersMatched(orderPtr);
    orders.remove(orderPtr);
    limitOrderBook.deleteOrder(orderPtr);
}

void MarketPlayer::deleteActiveOrder(std::shared_ptr<Order> orderPtr) {
    bool isBuy {orderPtr -> isBuy};
    std::list<std::shared_ptr<Order>> & orders = isBuy ? buyOrders : sellOrders;
    updatePortfolioIfOrdersMatched(orderPtr);
    orders.remove(orderPtr);
}

void MarketPlayer::deleteAllBuyOrders(LimitOrderBook & limitOrderBook) {
    for (auto orderPtr: buyOrders) {
        updatePortfolioIfOrdersMatched(orderPtr);
        limitOrderBook.deleteOrder(orderPtr);
    }
    buyOrders.clear();
}

void MarketPlayer::deleteAllSellOrders(LimitOrderBook & limitOrderBook) {
    for (auto orderPtr: sellOrders) {
        updatePortfolioIfOrdersMatched(orderPtr);
        limitOrderBook.deleteOrder(orderPtr);
    }
    sellOrders.clear();
}

void MarketPlayer::deleteAllOrders(LimitOrderBook & limitOrderBook) {
    deleteAllBuyOrders(limitOrderBook);
    deleteAllSellOrders(limitOrderBook);
}


/*********** helper methods ***********/


// need to implement this; would it be better to use a struct that doesn't initialize a random device at each call?
double MarketPlayer::generateTimestamp() const {
    std::random_device rd;
    auto distribution = std::uniform_real_distribution<double> (0., 1.);
    return connectionSpeed + distribution(rd);
}

void MarketPlayer::updateActualPortfolio(double diffPrice, double diffVolume, double sold) {
    double diffCash {diffPrice * diffVolume * sold};
    diffVolume *= sold * -1.;
    Portfolio diffPortfolio(diffCash, diffVolume);
    actualPortfolio += diffPortfolio;
}

void MarketPlayer::updatePortfolioIfOrdersMatched(double diffPrice, double diffVolume, bool isSell) {
    double sold {isSell ? 1. : -1.};
    double diffCash {diffPrice * diffVolume * sold};
    diffVolume *= sold * -1.;
    Portfolio diffPortfolio(diffCash, diffVolume);
    portfolioIfOrdersMatched += diffPortfolio;
}

void MarketPlayer::updatePortfolioIfOrdersMatched(std::shared_ptr<Order> orderPtr) {
    double diffPrice {orderPtr -> price};
    double diffVolume {orderPtr -> volume};
    updatePortfolioIfOrdersMatched(diffPrice, diffVolume, orderPtr -> isBuy);
}

bool MarketPlayer::canTrade(double price, double volume, bool isBuy) const {
    if (isBuy)
        return actualPortfolio.cash >= price * volume;
    else
        return actualPortfolio.stock >= volume;
}

void MarketPlayer::addOrder(std::shared_ptr<Order> orderPtr) {
    if (orderPtr -> isBuy)
        buyOrders.push_back(orderPtr);
    else
        sellOrders.push_back(orderPtr);
}

void MarketPlayer::prepareMarketOrder(std::shared_ptr<Order> orderPtr, LimitOrderBook & limitOrderBook) {
    deleteOldestOrders(orderPtr -> volume, limitOrderBook, orderPtr -> isBuy);
    orderPtr -> isBuy ? deleteAllSellOrders(limitOrderBook) : deleteAllBuyOrders(limitOrderBook);
}


/*********** trading behavioral methods ***********/

// perhaps I should not have the tradingLikelihood affect the price as much; it already affect too many things

double MarketPlayer::computeProbabilityOfTrading(LimitOrderBook & limitOrderBook, double news) const {
    double spread {limitOrderBook.getSpread()};
    double newsImpact {newsResponsiveness * news};
    double spreadImpact {connectionSpeed * spread};
    double imbalanceImpact {limitOrderBook.getVolumeImbalance() * VOLUME_PRICE_CHANGE_FACTOR};
    double priceDiff {limitOrderBook.get_VWAP() < LOWER_PRICE_LIMIT ? 0. : limitOrderBook.get_VWAP() - limitOrderBook.get_mid_price()};
    double priceImpact {priceDiff * PRICE_CHANGE_FACTOR};

    // 0.5 corresponds to no trading
    double totalImpact {newsImpact + spreadImpact + imbalanceImpact + priceImpact - riskAversion};

    // using a sigmoid function to map the total impact to a probability
    double probability = 1.0 / (1.0 + std::exp(-totalImpact));

    return probability;
}

bool MarketPlayer::determineIfLimitOrder(double tradingLikelihood, LimitOrderBook & limitOrderBook) const {
    return tradingLikelihood > 0.2 && tradingLikelihood < 0.8;
}

bool MarketPlayer::determineIfBuyOrSell(double tradingLikelihood, LimitOrderBook & limitOrderBook) const {
    return tradingLikelihood > 0.5;
}

double MarketPlayer::computeCashToReceive(double tradingLikelihood, LimitOrderBook & limitOrderBook) const {
    double proportion {(1 - tradingLikelihood) * (.5 - riskAversion)};
    return portfolioIfOrdersMatched.stock * limitOrderBook.get_mid_price() * proportion;
}

double MarketPlayer::computeCashToSpend(double tradingLikelihood, LimitOrderBook & limitOrderBook) const {
    double proportion {tradingLikelihood * (.5 - riskAversion)};
    return portfolioIfOrdersMatched.cash * proportion;
}

double MarketPlayer::computeStockToSell(double tradingLikelihood, LimitOrderBook & limitOrderBook, double price) const {
    double proportion {(1 - tradingLikelihood) * (.5 - riskAversion)};
    return portfolioIfOrdersMatched.stock * proportion;
}

double MarketPlayer::computeStockToBuy(double tradingLikelihood, LimitOrderBook & limitOrderBook, double price) const {
    double proportion {tradingLikelihood * (.5 - riskAversion)};
    return portfolioIfOrdersMatched.cash * proportion / price;
}

// Perhaps I should not have tradingLikelihood affect the price as much; it already affect too many things
double MarketPlayer::computePrice(double tradingLikelihood, LimitOrderBook & limitOrderBook, bool isBuy) const {
    double price {0.0};
    double spread {limitOrderBook.getSpread()};

    if (isBuy) {
        if (limitOrderBook.hasBidOrders()) {
            price = limitOrderBook.get_bid();
        } else {
            price = limitOrderBook.get_mid_price();
        }
    } else {
        if (limitOrderBook.hasAskOrders()) {
            price = limitOrderBook.get_ask();
        } else {
            price = limitOrderBook.get_mid_price();
        }
    }

    // Adjust the price based on tradingLikelihood, and riskAversion
    if (isBuy) { // increase the price the more they want to buy
        price *= (1. + (2. * tradingLikelihood  - riskAversion) * (1. + PRICE_CHANGE_FACTOR * spread));
    } else { // decrease the price the more they want to sell
        price *= (1. - (2. * tradingLikelihood  - riskAversion) * (1. + PRICE_CHANGE_FACTOR * spread));
    }

    return price;
}


double MarketPlayer::computeVolume(double tradingLikelihood, LimitOrderBook & limitOrderBook,
                                    bool isBuy, double price) const {
    if (price <= LOWER_PRICE_LIMIT) // extra check to make sure that low priced orders are not placed
        return 0.0;

    double ans {0.};

    if (isBuy)
        ans = computeCashToSpend(tradingLikelihood, limitOrderBook) / price;
    else
        ans =  computeCashToReceive(tradingLikelihood, limitOrderBook) / price;

    if (ans < LOWER_VOLUME_LIMIT)
        return 0.0;

    return ans;
}

/*********** trading functions ***********/

// perhaps trade should belong to the market, or not a friend of the class
// thing is I don't know how else to get the shared pointer to the order constructor
std::shared_ptr<Order> trade(double news, LimitOrderBook & limitOrderBook,
                            std::shared_ptr<MarketPlayer> traderPtr) {

    double tradingProbability = traderPtr -> computeProbabilityOfTrading(limitOrderBook, news);

    // if the trader is not likely to trade, return nullptr
    if (tradingProbability > UPPER_TRADING_LIMIT  && tradingProbability < LOWER_TRADING_LIMIT)
        return nullptr;

    bool buy {traderPtr -> determineIfBuyOrSell(tradingProbability, limitOrderBook)};
    double price {traderPtr -> computePrice(tradingProbability, limitOrderBook, buy)};
    // if the price is too low, return nullptr
    if (price <= LOWER_PRICE_LIMIT)
        return nullptr;

    double volume {traderPtr -> computeVolume(tradingProbability, limitOrderBook, buy, price)};
    // if the volume is too low, return nullptr
    if (volume <= LOWER_VOLUME_LIMIT)
        return nullptr;

    // check whether the trader can afford to place the order
    double cost {price * volume};
    if (buy && cost > traderPtr -> getCash())
        return nullptr;

    bool isLimit {traderPtr -> determineIfLimitOrder(tradingProbability, limitOrderBook)};

    auto orderPtr = std::make_shared<Order>(price, volume, buy, isLimit, traderPtr);

    // update the trader's hypothetical portfolio
    if (isLimit) {
        traderPtr -> updatePortfolioIfOrdersMatched(price, volume, buy ? -1 : 1);
    }
    else {
        traderPtr -> prepareMarketOrder(orderPtr, limitOrderBook);
    }

    return orderPtr;
}
