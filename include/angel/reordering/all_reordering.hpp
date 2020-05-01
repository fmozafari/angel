#include <vector>

namespace angel
{

class AllReordering
{
public:
    using order = std::vector<uint32_t>;

public:
    explicit AllReordering() {}

    std::vector<order> run(uint32_t var_count)
    {
        std::vector<order> all_orders;
        order orders_init;
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