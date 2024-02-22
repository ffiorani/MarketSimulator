#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "Market.h"

#include <fstream>
#include <vector>

/**
 * @brief Write a vector to a file
 *
 * @param vec
 * @param filename
 */
void writeVectorToFile(const std::vector<double>& vec, const std::string& filename) {
    std::ofstream file(filename);

    for (const auto& val : vec) {
        file << val << '\n';
    }

    file.close();
}

/**
 * @brief
 * Main function. Simulates the market and writes the price history to a file.
 * @return int
 */
int main() {
    const size_t numTraders {3};
    const size_t numSteps {100};

    Market market(numTraders);

    std::vector<double> priceHistory;

    market.simulateMarket(numSteps, priceHistory);

    writeVectorToFile(priceHistory, "priceHistory.csv");
    writeVectorToFile(market.getCashDistribution(), "cashDistribution.csv");
    writeVectorToFile(market.getSharesDistribution(), "sharesDistribution.csv");
    writeVectorToFile(market.getWealthDistribution(), "wealthDistribution.csv");

    std::cout << market;

    return 0;
}
