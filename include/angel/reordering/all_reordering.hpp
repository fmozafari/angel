#include <vector>

namespace angel
{

class AllOrdering
{
public:
    explicit AllOrdering() {}

    std::vector<std::vector<uint32_t>> run(uint32_t var_count)
    {
        std::vector<std::vector<uint32_t>> all_orders;
        std::vector<uint32_t> orders_init;
        for(auto i=0u; i<var_count; i++)
            orders_init.emplace_back(i);
        do
        {
            all_orders.emplace_back(orders_init);
        } while(std::next_permutation(orders_init.begin(), orders_init.end()));

        return all_orders;
    }

};

} /// namespace angel end