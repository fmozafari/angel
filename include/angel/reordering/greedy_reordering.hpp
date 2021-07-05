#include <algorithm>
#include <chrono>
#include <random>
#include <vector>
#include <optional>
#include <kitty/kitty.hpp>

namespace angel
{

class greedy_reordering
{
public:
  template<typename Fn>
  void foreach_reordering( kitty::dynamic_truth_table tt, Fn&& fn, std::optional<uint32_t> initial_cost = std::nullopt ) const
  {
    kitty::dynamic_truth_table first_tt{tt};

    uint32_t const num_variables = tt.num_vars();

    std::vector<uint32_t> perm;
    for ( int32_t i = tt.num_vars()-1; i >= 0; i-- )
    {
      perm.emplace_back( i );
    }

    fn( first_tt, perm );

    uint32_t best_cost = initial_cost ? *initial_cost : fn( tt , perm);
    bool forward = true;
    bool improvement = true;

    while ( improvement )
    {
      improvement = false;

      for ( int32_t i = forward ? 0 : num_variables - 2; forward ? i < static_cast<int32_t>( num_variables - 1 ) : i >= 0; forward ? ++i : --i )
      {
        bool local_improvement = false;
        kitty::dynamic_truth_table const next_tt = kitty::swap( tt, perm[i], perm[i + 1] );

        if ( next_tt == first_tt || next_tt == tt )
          continue;

        std::swap( perm[i], perm[i + 1] );
        uint32_t const cost = fn( next_tt, perm );
        if ( cost < best_cost )
        {
          best_cost = cost;
          tt = next_tt;
          //std::swap( perm[i], perm[i + 1] );
          local_improvement = true;
        }
        else
          std::swap( perm[i], perm[i + 1] );

        if ( local_improvement )
        {
          improvement = true;
        }
      }

      forward = !forward;
    }
  }
}; 

} // namespace angel end
