#include <iostream>
#include <random>
#include <memory>
#include "MarketPlayer.h"


const double INITIAL_WEALTH{1};


MarketPlayer::MarketPlayer() : totalWealth(INITIAL_WEALTH), id(numCurrTraders++) {
    std::random_device rd;
    auto distribution = std::uniform_real_distribution<double> (-1., 1.);
    portfolio.first = INITIAL_WEALTH * distribution(rd);
    portfolio.second = INITIAL_WEALTH - portfolio.first;
    newsResponsiveness = distribution(rd);
    activeOrder = nullptr;
}

double MarketPlayer::getTotalWealth() const {  return totalWealth; }
double MarketPlayer::getCash() const {  return portfolio.first; }
int MarketPlayer::getId() const { return id; }
double MarketPlayer::getShares() const  { return portfolio.second; }
std::shared_ptr<LimitOrder> MarketPlayer::getActiveOrder() { return activeOrder; }


/*  Deletes an active order of @amountOfShares, if present;
    currently only deletes the shared ptr stored by the @MarketPlayer,
    not the one held by the @LimitOrderBook.
    Returns true if the Market Order has been completely deleted */

bool MarketPlayer::deleteActiveOrder(double amountOfShares) {
    if (activeOrder == nullptr) {
        std::cerr << "Trader %d has no current active order \n".format(id);
        throw NoPointerException();
    }

    if (activeOrder -> second <= amountOfShares) {
        activeOrder = nullptr;
        return true;
    } else {
        activeOrder -> second -= amountOfShares;
        return false;
    }
}


void MarketPlayer::trade(LimitOrderBook & limitOrderBook, double news) {
    double tradingLikelihood {this -> computeProbabilityOfTrading(limitOrderBook, news)};
    // determine if buy or sell or do nothing
    if (this -> determineIfMarketOrder(tradingLikelihood)) {
        placeMarketOrder(tradingLikelihood, limitOrderBook);
    } else {
        placeLimitOrder(tradingLikelihood, limitOrderBook);
    }
}


void MarketPlayer::placeMarketOrder(double, LimitOrderBook &) {
    double volume {this -> computeVolume(tradingLikelihood, limitOrderBook)};
    double price {this -> computePrice(tradingLikelihood, limitOrderBook)};
    MarketOrder marketOrder(price, volume);
    // actually need to check if the there is already an active order, and modify that before placing anything else... need to restructure this
}
