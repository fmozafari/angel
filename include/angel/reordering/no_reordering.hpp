#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

namespace angel
{

class NoReordering
{
public:
    using order = std::vector<uint32_t>;

public:
    explicit NoReordering()
    {
    }

    std::vector<order> run(uint32_t var_count) const
    {
        order current_order;
        for (auto i = 0u; i < var_count; i++)
        {
            current_order.emplace_back(i);
        }
        std::reverse(current_order.begin(), current_order.end());
        return {current_order};
    }

    using dependencies_t = std::map<uint32_t, std::vector<std::pair<std::string, std::vector<uint32_t>>>>;
    bool compute_next_order(order& current_order, uint32_t var_count, dependencies_t deps)
    {
        return false;
    }
};

} // namespace angel
