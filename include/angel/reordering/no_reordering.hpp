#include <algorithm>
#include <vector>
#include <optional>
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
    std::vector<uint32_t> perm;
    for ( int32_t i = tt.num_vars()-1; i >= 0; i-- )
    {
      perm.emplace_back( i );
    }

    fn( tt, perm );
  }
}; 

} // namespace angel
