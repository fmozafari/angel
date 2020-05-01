#include <algorithm>
#include <chrono>
#include <random>
#include <vector>
#include <kitty/kitty.hpp>

namespace angel
{

class SiftReordering
{
public:
    using order = std::vector<uint32_t>;

public:
    explicit SiftReordering(kitty::dynamic_truth_table func) : tt(func)
    {
    }

    std::vector<order> run(uint32_t num_vars)
    {
        std::vector<order> orders;
        order perm( num_vars );
        std::iota( perm.begin(), perm.end(), 0u );
        std::reverse(perm.begin(), perm.end());
        orders.emplace_back(perm);
        auto forward = true;
        auto improvement = true;

        while ( improvement )
        {
            improvement = false;

            for ( int i = forward ? 0 : num_vars - 2; forward ? i < static_cast<int>( num_vars - 1 ) : i >= 0; forward ? ++i : --i )
            {
                auto local_improvement = false;
                const auto next_t = kitty::swap( tt, perm[i], perm[i + 1] );
                if ( next_t < tt )
                {
                    tt = next_t;
                    kitty::print_binary(tt);
                    std::cout<<std::endl;
                    std::swap( perm[i], perm[i + 1] );
                    orders.emplace_back(perm);
                    local_improvement = true;
                }

                if ( local_improvement )
                {
                    improvement = true;
                }
            }

            forward = !forward;
        }        
        return orders;
    }

private:
kitty::dynamic_truth_table tt;

};

} // namespace angel
