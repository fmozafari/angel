#include <vector>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <random>

namespace angel
{

class RandomOrdering
{
private:
uint32_t num = 1;
public:
    explicit RandomOrdering() {}
    explicit RandomOrdering(uint32_t n): num(n) {}

    std::vector<std::vector<uint32_t>> run(uint32_t var_count)
    {
        std::vector<std::vector<uint32_t>> all_orders;
        std::vector<uint32_t> orders_init;
        for(auto i=0u; i<var_count; i++)
            orders_init.emplace_back(i);

        // std::random_device rd;
        // std::mt19937 g(rd());
        // obtain a time-based seed:
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine e(seed);

        for(auto i=0u; i<num; i++)
        {
            std::shuffle(orders_init.begin(), orders_init.end(), e);
            all_orders.emplace_back(orders_init);
        }
        

        return all_orders;
    }

};

} /// namespace angel end