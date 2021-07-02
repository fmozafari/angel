#include <algorithm>
#include <vector>
#include <optional>
#include <kitty/kitty.hpp>

namespace angel
{

class exhaustive_reordering
{
public:
  template<typename Fn>
  void foreach_reordering( kitty::dynamic_truth_table const& tt, Fn&& fn, std::optional<uint32_t> initial_cost = std::nullopt ) const
  {
    (void)initial_cost;
    
    std::vector<uint32_t> perm;
    for ( int32_t i = tt.num_vars()-1; i >= 0; i-- )
    {
      perm.emplace_back( i );
    }

    do
    {
      kitty::dynamic_truth_table tt_( tt );
      angel::reordering_on_tt_inplace( tt_, perm );
      fn( tt_, perm );
    }
    while ( std::next_permutation( std::begin( perm ), std::end( perm ) ) );
  }
}; 

} /// namespace angel end