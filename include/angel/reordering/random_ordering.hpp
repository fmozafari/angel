#include <vector>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <random>

namespace angel
{

class RandomOrdering
{
public:
    explicit RandomOrdering() {}

    std::vector<std::vector<uint32_t>> run(uint32_t var_count)
    {
        std::vector<std::vector<uint32_t>> all_orders;
        std::vector<uint32_t> orders_init;
        for(auto i=0u; i<var_count; i++)
            orders_init.emplace_back(i);

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(orders_init.begin(), orders_init.end(), g);
        all_orders.emplace_back(orders_init);

        return all_orders;
    }

};

} /// namespace angel end