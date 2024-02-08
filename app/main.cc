#include <iostream>

#include "Market.h"

int main() {
    const size_t numTraders {100};
    const size_t numSteps {100};

    Market market(numTraders);

    market.simulateMarket(numSteps);

    std::cout << "The mid price is: " << market.printMarketData() << std::endl;

    return 0;
}
