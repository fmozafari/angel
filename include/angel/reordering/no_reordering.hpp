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

    std::vector<order> run(uint32_t num_vars) const
    {
        order current_order;
        for (auto i = 0u; i < num_vars; i++)
        {
            current_order.emplace_back(i);
        }
        std::reverse(current_order.begin(), current_order.end());
        return {current_order};
    }
};

} // namespace angel
