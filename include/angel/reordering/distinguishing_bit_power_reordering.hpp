#include <vector>
#include <kitty/dynamic_truth_table.hpp>

namespace angel
{

class DistinguishingBitPowerReordering
{
public:
    using order_t = std::vector<uint32_t>;

    struct column
    {
        kitty::partial_truth_table tt;
        uint64_t entropy;
        uint32_t index;
    };

public:
    explicit DistinguishingBitPowerReordering() {}

    std::vector<order_t> run(kitty::dynamic_truth_table const &function)
    {
        std::vector<column> columns(function.num_vars());
        for (auto i = 0u; i < columns.size(); ++i)
        {
            columns[i].index = i;
        }

        kitty::dynamic_truth_table minterm(function.num_vars());
        for (auto const &m : get_minterms(function))
        {
            minterm._bits[0] = m;

            // std::cout << "minterm: " << m << ' '; kitty::print_binary( minterm ); std::cout << std::endl;

            for (auto i = 0; i < minterm.num_vars(); ++i)
            {
                // std::cout << i << ' ' << kitty::get_bit( minterm, i ) << std::endl;
                columns[i].tt.add_bit(kitty::get_bit(minterm, i));
            }
        }

        std::vector<order_t> orders;
        order_t order(columns.size());

        std::vector<column> columns_copy;
        for (auto i = 0u; i < columns.size(); ++i)
        {
            columns_copy = columns;

            auto &target = columns_copy[i];
            target.entropy = std::numeric_limits<uint64_t>::max();

            // std::cout << target.index << std::endl;

            for (auto j = 0u; j < columns_copy.size(); ++j)
            {
                if (i == j)
                    continue;

                columns_copy[j].entropy = kitty::relative_distinguishing_power(columns_copy[j].tt, target.tt);
            }

            /* sort the remaining columns by entropy (highest entropy first) */
            std::sort(std::rbegin(columns_copy), std::rend(columns_copy), [&](auto const &a, auto const &b) {
                return a.entropy < b.entropy || (a.entropy == b.entropy && a.index < b.index);
            });

            for (auto j = 0u; j < columns_copy.size(); ++j)
            {
                // std::cout << "entropy: " << columns_copy[j].index << ' ' << columns_copy[j].entropy << std::endl;
                order[j] = columns_copy[j].index;
            }
            // std::cout << std::endl;

            orders.emplace_back(order);
        }

        return orders;
    }

}; /// class DistinguishingBitPowerReordering end

} /// namespace angel