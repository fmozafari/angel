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
    fn( tt );
  }
}; 

} // namespace angel
