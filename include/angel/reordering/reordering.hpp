#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

#include <kitty/kitty.hpp>

namespace angel
{

class no_reordering
{
public:
  template<typename Fn>
  void foreach_reordering( kitty::dynamic_truth_table const& tt, Fn&& fn, std::optional<uint32_t> initial_cost = std::nullopt ) const
  {
    (void)initial_cost;
    fn( tt );
  }
}; /* no_reordering */

class exhaustive_reordering
{
public:
  template<typename Fn>
  void foreach_reordering( kitty::dynamic_truth_table const& tt, Fn&& fn, std::optional<uint32_t> initial_cost = std::nullopt ) const
  {
    (void)initial_cost;
    
    std::vector<uint32_t> perm;
    for ( auto i = 0u; i < tt.num_vars(); ++i )
    {
      perm.emplace_back( i );
    }

    do
    {
      kitty::dynamic_truth_table tt_( tt );
      angel::reordering_on_tt_inplace( tt_, perm );
      fn( tt_ );
    }
    while ( std::next_permutation( std::begin( perm ), std::end( perm ) ) );
  }
}; /* exhaustive_reordering */

class random_reordering
{
public:
  explicit random_reordering( uint64_t seed, uint64_t num_reordering )
    : seed( seed )
    , num_reordering( num_reordering )
  {
  }
  
  template<typename Fn>
  void foreach_reordering( kitty::dynamic_truth_table const& tt, Fn&& fn, std::optional<uint32_t> initial_cost = std::nullopt ) const
  {
    (void)initial_cost;

    fn( tt );

    if ( num_reordering == 0u )
      return;

    std::vector<uint32_t> perm;
    for ( auto i = 0u; i < tt.num_vars(); ++i )
    {
      perm.emplace_back( i );
    }
    
    std::default_random_engine random_engine( seed );
    std::vector<std::vector<uint32_t>> orders;
    for ( auto i = 0u; i < num_reordering; ++i )
    {
      std::shuffle( std::begin( perm ), std::end( perm ), random_engine );

      if ( std::find( std::begin( orders ), std::end( orders ), perm ) == orders.end() )
      {
        kitty::dynamic_truth_table tt_( tt );
        angel::reordering_on_tt_inplace( tt_, perm );

        if ( tt != tt_ )
        {
          fn( tt_ );
          orders.emplace_back( perm );
          std::sort( std::begin( perm ), std::end( perm ) );
        }
      }
    }
  }

protected:
  uint64_t seed;
  uint64_t num_reordering;
}; /* random_reordering */

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

        if ( tt == first_tt || next_tt == tt )
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
}; /* greedy_reordering */

} // angel
