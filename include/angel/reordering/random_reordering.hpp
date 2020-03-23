#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

namespace angel
{

class RandomReordering
{
public:
    using order = std::vector<uint32_t>;

public:
    explicit RandomReordering(uint32_t num_orders)
        : seed(std::chrono::system_clock::now().time_since_epoch().count()), num_orders(num_orders)
    {
    }

    explicit RandomReordering(uint64_t seed, uint32_t num_orders)
        : seed(seed), num_orders(num_orders)
    {
    }

    std::vector<order> run(uint32_t num_vars) const
    {
        order current_order;
        for (auto i = 0u; i < num_vars; ++i)
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

    using dependencies_t = std::map<uint32_t, std::vector<std::pair<std::string, std::vector<uint32_t>>>>;
    bool compute_next_order(order& current_order, uint32_t var_count, dependencies_t deps)
    {
        return false;
    }

private:
    uint64_t seed;
    uint32_t num_orders;
};

} // namespace angel
