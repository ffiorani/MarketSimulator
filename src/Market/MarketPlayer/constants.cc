const double INITIAL_WEALTH = 1.0;
const double LOWER_VOLUME_LIMIT = 0.001;
const double LOWER_WEALTH_LIMIT = 0.000001;

/**
 * @brief
 * constant that controls whether the trader wants to place an order. The baseline is 0.5: the trader is indifferent.
 * Used in the trade function in MarketPlayer.cc.
 */
const double LOWER_TRADING_LIMIT = 0.55;
const double UPPER_TRADING_LIMIT = 0.45;
