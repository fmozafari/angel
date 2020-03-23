#include <vector>
#include <map>
#include <chrono>
#include <random>

namespace angel
{

class ConsideringDepsReordering
{
public:
    using order = std::vector<uint32_t>;
    using dependencies_t = std::map<uint32_t, std::vector<std::pair<std::string, std::vector<uint32_t>>>>;

public:
    explicit ConsideringDepsReordering(uint32_t num_orders)
        : seed(std::chrono::system_clock::now().time_since_epoch().count()), num_orders(num_orders)
    {
    }

    explicit ConsideringDepsReordering(uint64_t seed, uint32_t num_orders)
        : seed(seed), num_orders(num_orders)
    {
    }

    std::vector<order> run(uint32_t var_count)
    {
        order current_order;
        for (auto i = 0u; i < var_count; ++i)
        {
            current_order.emplace_back(i);
        }

        std::default_random_engine random_engine(seed);

        std::vector<order> orders;
        for (auto i = 0u; i < num_orders; ++i)
        {
            std::shuffle(current_order.begin(), current_order.end(), random_engine);
            orders.emplace_back(current_order);
        }
        return orders;
    }

    bool compute_next_order(order& current_order, uint32_t var_count, dependencies_t deps)
    {
        if(deps.size()==0)
            return false;

        for(auto i=0u; i<var_count; i++)
        {
            auto it = deps.find(i);
            if(it==deps.end())
                continue;

            if( (deps[i][0].first == "and" || deps[i][0].first == "nand" || deps[i][0].first == "or" || deps[i][0].first == "nor") 
            && deps[i][0].second.size()<(var_count-1) )
            {
                current_order.emplace_back(i);
            }
        }

        for (auto i=0u ; i<var_count; i++)
        {
            auto it = std::find(current_order.begin(), current_order.end(), i);
            if(it == current_order.end())
            {
                current_order.emplace_back(i);
            }
        }
        std::reverse(current_order.begin(), current_order.end());
        return true;
    }

private:
    uint64_t seed;
    uint32_t num_orders;

};

} /// namespace angel end