#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "Market.h"

#include <fstream>
#include <vector>

void writeVectorToFile(const std::vector<double>& vec, const std::string& filename) {
    std::ofstream file(filename);

    for (const auto& val : vec) {
        file << val << '\n';
    }

    file.close();
}

int main() {
    const size_t numTraders {100};
    const size_t numSteps {100};

    Market market(numTraders);

    std::vector<double> priceHistory;

    market.simulateMarket(numSteps, priceHistory);

    writeVectorToFile(priceHistory, "priceHistory.csv");
    std::cout << market;

    return 0;
}
