#pragma once

#include <memory>
#include <list>

#include "constants.cc"

class LimitOrderBook;
struct Order;


/**
 * @brief Struct that contains the portfolio of a MarketPlayer. It included the amount of cash the trader
 * possesses, as well as the amount of stock they own.
 *
 */
struct Portfolio {
    double cash;
    double stock;
    Portfolio();
    Portfolio(double, double);
    Portfolio(const Portfolio &);
    Portfolio & operator=(const Portfolio &);
    Portfolio operator+(const Portfolio &) const;
    Portfolio & operator+=(const Portfolio &);
};


/**
 * @brief Class modelling traders in the market. Each trader has a unique id, a portfolio, a total wealth,
 * a connection speed and a news responsiveness. The trader can place orders in the market, and their
 * portfolio is updated when their orders are matched. The trader can also delete their active orders.
 * The trader follows a set of rules to determine what orders to place, based on the information available
 * in the LimitOrderBook and the news.
 *
 * Traders will avoid bankruptcy by not placing orders that would make them go bankrupt at the time. However, as the
 * price changes and because market orders are placed, traders can go bankrupt. For now, the market will not stop
 * when a trader goes bankrupt, but the trader will not be able to place any more orders. The orders they have currently
 * on the market that they cannot match will only disappear when
 * 1) they are partially matched by some other order being placed
 * 2) the trader decides to delete them in order to place a new order
 * 3) the trader wants the portfolioIfOrdersMatched, which determines whether they can trade or not, to be updated.
 *
 * A possible reason for bankruptcy other than price change is the fact that currently Market orders will be matched at
 * at market price rather than the trader's estimation, which is currently done in the same way as Limit orders.
 *
 *
 */
class MarketPlayer {
    private:
        /**
         * @brief Number of traders that have currently been created. Used to assign unique ids to each trader, as
         * traders are not deleted from the market once created. Perhaps there is a better way to assign unique ids.
         *
         */
        static int numCurrTraders;

        /**
         * @brief Unique id of the trader. Used to distinguish between orders placed by different traders.
         *
         */
        int id;

        /**
         * @brief Portfolio of the trader. Contains the amount of cash and stock the trader possesses.
         *
         */
        Portfolio actualPortfolio;

        /**
         * @brief Portfolio of the trader if all their orders are matched. Used to avoid bankruptcy.
         *
         */
        Portfolio portfolioIfOrdersMatched;

        /**
         * @brief Connection speed of the trader.
         * Used to determine the probability of the trader placing an order in the market, as well as the time they
         * place the order within the time frame of the market.
         *
         */
        double connectionSpeed;

        /**
         * @brief Risk aversion of the trader.
         * Used to determine the amount of cash the trader is willing to spend on a stock, as well as the amount of
         * stock the trader is willing to buy or sell.
         *
         */
        double riskAversion;

        /**
         * @brief News responsiveness of the trader.
         * Models how much the trader is affected by the news when placing orders in the market.
         * It is a way to partially model risk aversion, as traders with a higher news responsiveness
         * will be more affected.
         *
         */
        double newsResponsiveness;


        /**
         * @brief Set of active sell orders of the trader.
         * As the trader can only place at most one order per time step, the orders about to be dequeued are the oldest.
         * This is our policy for order removal when a trader wants to delete an order. If an order is fully matched, or
         * the trader cannot afford to fully match their active limit order, the order is deleted through a vector
         * traversal, which is perhaps not the most efficient.
         *
         * Currently Market orders are also included in the active
         * orders if they are not fully matched. Should I have a separate queue for unmatched market orders?
         *
         */
        std::list<std::shared_ptr<Order>> buyOrders;


        /**
         * @brief Set of active sell orders of the trader.
         * As the trader can only place at most one order per time step, the orders about to be dequeued are the oldest.
         * This is our policy for order removal when a trader wants to delete an order. If an order is fully matched, or
         * the trader cannot afford to fully match their active limit order, the order is deleted through a vector
         * traversal, which is perhaps not the most efficient.
         *
         * Currently Market orders are also included in the active
         * orders if they are not fully matched. Should I have a separate queue for unmatched market orders?
         *
         */
        std::list<std::shared_ptr<Order>> sellOrders;

        /************* trading methods *************/

        /**
         * @brief Computes the probability of the trader placing an order in the market at a given time step.
         * The probability is determined by the news and information available in the @LimitOrderBook.
         * The higher the probability, the more likely the trader is to place an market order, higher volume or
         * worse priced limit order.
         *
         *
         * @param limitOrderBook LimitOrderBook
         * @param news  Double representing the intensity of the news
         */
        double computeProbabilityOfTrading(LimitOrderBook &, double) const;

        /**
         * @brief Helper function of computeVolume. Determines the amount of cash the trader is willing to spend on a stock, based on the news and
         * information available in the @LimitOrderBook and their portfolio.
         *
         * @param tradingLikelihood intensity of the willingness to trade
         * @param LimitOrderBook LimitOrderBook
         * @return double
         */
        double computeCashToSpend(double, LimitOrderBook &) const;

        /**
         * @brief Currently not used. Determines the amount of stock the trader is willing to buy, based on the news and
         * information available in the @LimitOrderBook and their portfolio.
         *
         * @param tradingLikelihood intensity of the willingness to trade
         * @param LimitOrderBook LimitOrderBook
         * @param price Price at which the trader is willing to buy
         * @return double
         */
        double computeStockToBuy(double, LimitOrderBook &, double) const;

        /**
         * @brief Helper function of ComputeVolume. Determines the amount of cash the trader is willing to receive for a stock, based on the news and
         * information available in the @LimitOrderBook and their portfolio.
         *
         * @param tradingLikelihood intensity of the willingness to trade
         * @param LimitOrderBook LimitOrderBook
         * @return double
         */
        double computeCashToReceive(double, LimitOrderBook &) const;

        /**
         * @brief Currently not used. Determines the amount of stock the trader is willing to sell, based on the news and
         * information available in the @LimitOrderBook and their portfolio.
         *
         * @param tradingLikelihood intensity of the willingness to trade
         * @param LimitOrderBook LimitOrderBook
         * @param price Price at which the trader is willing to sell
         * @return double
         */
        double computeStockToSell(double, LimitOrderBook &, double) const;

        /**
         * @brief Determines if the trader wants to buy or sell a stock, based on the news and
         * information available in the @LimitOrderBook and their portfolio.
         *
         * @param tradingLikelihood  intensity of the willingness to trade
         * @param LimitOrderBook LimitOrderBook
         * @return bool
         */
        bool determineIfBuyOrSell(double, LimitOrderBook &) const;

        /**
         * @brief Determines if the trader wants to place a limit or market order, based on the news and
         * information available in the @LimitOrderBook and their portfolio.
         *
         * @param tradingLikelihood intensity of the willingness to trade
         * @param LimitOrderBook LimitOrderBook
         * @return bool
         */
        bool determineIfLimitOrder(double, LimitOrderBook &) const;

        /**
         * @brief Determines the price of the limit order the trader wants to place, based on the news and
         * information available in the @LimitOrderBook and their portfolio.
         *
         * @param tradingLikelihood intensity of the willingness to trade
         * @param LimitOrderBook LimitOrderBook
         * @param isBuy Whether the trader wants to buy or sell
         * @return double
         */
        double computePrice(double, LimitOrderBook &, bool) const;

        /**
         * @brief Determines the volume of the limit order the trader wants to place, based on the news and
         * information available in the @LimitOrderBook and their portfolio and the price that the trader has already
         * decided to place the order at.
         *
         * @param tradingLikelihood intensity of the willingness to trade
         * @param LimitOrderBook LimitOrderBook
         * @param isBuy Whether the trader wants to buy or sell
         * @param price Price at which the trader wants to buy or sell
         * @return double
         */
        double computeVolume(double, LimitOrderBook &, bool, double) const;

        /**
         * @brief Helper function of trade.
         * If the trader wants to place a market order, or the orders of the opposite type must be deleted.
         * Moreover, the oldest orders of the trader of the same type are deleted by an equal amount of the volume
         * of the market order.
         *
         * @param tradingLikelihood intensity of the willingness to trade
         * @param LimitOrderBook LimitOrderBook
         */
        void prepareMarketOrder(std::shared_ptr<Order> orderPtr, LimitOrderBook & limitOrderBook);

        /************* delete order methods *************/

        /**
         * @brief Deletes all active orders of the trader.
         *  Currently does not affect price or decision making of other traders.
         */
        // currently not allowing traders to change or partially delete active orders, without placing an opposite limit order or a market order
        // bool determineLimitOrMarket(double, LimitOrderBook &);
        void deleteAllOrders(LimitOrderBook &);

        /**
         * @brief Deletes all active buy orders of the trader.
         *  Currently does not affect price or decision making of other traders.
         */
        void deleteAllBuyOrders(LimitOrderBook &);

        /**
         * @brief Deletes all active sell orders of the trader.
         *  Currently does not affect price or decision making of other traders.
         */
        void deleteAllSellOrders(LimitOrderBook &);

        /**
         * @brief Helper function of prepareMarketOrder.
         * Deletes the oldest orders of the trader of an equal amount of the volume of the market order of the same
         * type they want to place. Returns true if there were enough unmatched orders to achieve this, false otherwise.
         * Currently does not affect price or decision making of other traders.
         *
         * @param volume Volume of the market order they want to place
         * @param limitOrderBook LimitOrderBook
         * @param isBuy type of the market order they want to place
         * @return bool
         */
        bool deleteOldestOrders(double volume, LimitOrderBook & limitOrderBook, bool isBuy);

        /**
         * @brief Deletes an active order of @amountOfShares, if present;
         * currently only deletes the shared ptr stored by the @MarketPlayer,
         * and the one held by the @LimitOrderBook.
         * Returns true if the Market Order has been completely deleted.
         * Currently does not affect price or decision making of other traders.
         *
         * @param orderPtr Pointer to the order being deleted
         * @param limitOrderBook LimitOrderBook
         */
        void deleteActiveOrder(std::shared_ptr<Order>, LimitOrderBook &);

    public:
        /**
         * @brief Deletes an active order of @amountOfShares, if present;
         * currently only deletes the shared ptr stored by the @MarketPlayer,
         * not the one held by the @LimitOrderBook.
         * Returns true if the Market Order has been completely deleted.
         * Currently does not affect price or decision making of other traders.
         *
         * @param orderPtr Pointer to the order being deleted
         * @param limitOrderBook LimitOrderBook
         */
        void deleteActiveOrder(std::shared_ptr<Order>);


        /************* constructors *************/


        /**
         * @brief Construct a new Market Player object. Initializes the total wealth of the trader as determined by the
         * INITIAL_WEALTH constant, and randomly assigns the amount of cash and stock in the trader's portfolio. Also
         * randomly assigns the connection speed and news responsiveness of the trader. Finally, assigns a unique id to
         * the trader and increments the number of traders that have been created.
         *
         */
        MarketPlayer();

        /**
         * @brief Destroy the Market Player object. Decrements the number of traders that have been created.
         *
         */
        ~MarketPlayer() { numCurrTraders--; }


        /************* getters *************/

        /**
         * @brief Get the total wealth of the trader.
         *
         * @return double
         */
        double getTotalWealth(const LimitOrderBook &) const;

        /**
         * @brief Get the wealth of the trader if all their orders are matched.
         *
         * @return double
         */
        double getWealthIfOrdersMatched(const LimitOrderBook &) const;

        /**
         * @brief Get the amount of cash in the trader's portfolio.
         *
         * @return double
         */
        double getCash() const;

        /**
         * @brief Get the amount of cash in the trader's portfolio if all their orders are matched.
         *
         * @return double
         */
        double getCashIfOrdersMatched() const;

        /**
         * @brief Get the unique id of the trader.
         *
         * @return int
         */
        int getId() const;

        /**
         * @brief Get the amount of stock in the trader's portfolio.
         *
         * @return double
         */
        double getShares() const;

        /**
         * @brief Get the amount of stock in the trader's portfolio if all their orders are matched.
         *
         * @return double
         */
        double getSharesIfOrdersMatched() const;

        /**
         * @brief Get the set of active unmatched buy orders of the trader. Currently not used. Includes Market Orders.
         *
         * @return std::list<std::shared_ptr<Order>>
         */
        std::list<std::shared_ptr<Order>> const & getBuyOrders() const;

        /**
         * @brief Get the set of active unmatched sell orders of the trader. Currently not used. Includes Market Orders.
         *
         * @return std::list<std::shared_ptr<Order>>
         */
        std::list<std::shared_ptr<Order>> const & getSellOrders() const;

        /**
         * @brief Get the number of traders that have been created.
         *
         * @return int
         */
        int getNumCurrTraders() const;

        /************* helper functions *************/

        /**
         * @brief Generates a timestamp for the trader based on their @connectionSpeed.
         * The timestamp is used to determine when the trader places an order within the time frame of the market.
         *
         * @return double
         */
        double generateTimestamp() const;

        /**
         * @brief Determines if the trader can afford the transaction about to take place.
         * Used by WalkTheBook. If false, only partial matching occurs.
         *
         * @param price Price at which the transaction is about to take place.
         * @param volume Volume of the transaction.
         * @param isBuy Whether the transaction is a buy or sell.
         * @return bool
         */
        bool canTrade(double, double, bool) const;

        void addOrder(std::shared_ptr<Order> orderPtr);

        /**
         * @brief Updates the portfolio of the trader, based on the transaction that has been made in the market.
         *
         * @param diffPrice Price of the Limit order being placed, or the order being deleted.
         * @param diffVolume Volume of the Limit order being placed, or the order being deleted.
         * @param isSell whether the Limit order being placed is a sell order,
         * or the order being deleted is a buy order.
         */
        void updateActualPortfolio(double, double, double);

        /**
         * @brief Updates the hypothetical portfolio of the trader when a Limit Order is placed. Used in
         * the delete methods, the trade method, and the LimitOrderBook::matchOrders method.
         *
         * @param diffPrice Price of the Limit order being placed, or the order being deleted.
         * @param diffVolume Volume of the Limit order being placed, or the order being deleted.
         * @param isSell whether the Limit order being placed is a sell order,
         * or the order being deleted is a buy order.
         */
        void updatePortfolioIfOrdersMatched(double, double, bool);

        /**
         * @brief Updates the hypothetical portfolio of the trader when an order is deleted.
         *
         * @param orderPtr Order being deleted or matched.
         */
        void updatePortfolioIfOrdersMatched(std::shared_ptr<Order> orderPtr);

        /************* trading functions *************/

        /**
         * @brief Takes the @traderPtr as input, who decides what order they want to place
         * based on the information available in the @LimitOrderBook and the @news. Also checks whether the order would
         * bankrupt the trader, and if so, returns a nullptr. Otherwise, returns the order they want to place.
         * @return
         * returns a shared pointer to the order they want to place or nullptr if they would go bankrupt or if the order
         * is not valid.
         *
         * @param news Double representing the intensity of the news - currently not used in the implementation
         * @param LimitOrderBook LimitOrderBook
         * @param traderPtr Pointer to the trader placing the order
         * @return std::shared_ptr<Order>
         */
        friend std::shared_ptr<Order> trade(double news, LimitOrderBook & LimitOrderBook,
                                            std::shared_ptr<MarketPlayer> traderPtr);
};
