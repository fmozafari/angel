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

    using dependencies_t = std::map<uint32_t, std::vector<std::pair<std::string, std::vector<uint32_t>>>>;
    bool compute_next_order(order& current_order, uint32_t var_count, dependencies_t deps)
    {
        return false;
    }

};

} /// namespace angel end