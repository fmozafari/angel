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
}; 

} /// namespace angel end