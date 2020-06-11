#include <algorithm>
#include <chrono>
#include <random>
#include <vector>
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

    fn( first_tt );

    std::vector<uint8_t> perm( num_variables );
    std::iota( perm.begin(), perm.end(), 0u );
    std::reverse( perm.begin(), perm.end() );

    uint32_t best_cost = initial_cost ? *initial_cost : fn( tt );
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

        uint32_t const cost = fn( next_tt );
        if ( cost < best_cost )
        {
          best_cost = cost;
          tt = next_tt;
          std::swap( perm[i], perm[i + 1] );
          local_improvement = true;
        }

        // fmt::print( "[i] function = {} reordered to {}\n", kitty::to_hex( tt ), kitty::to_hex( next_tt ) );

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
