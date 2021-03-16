#include <algorithm>
#include <chrono>
#include <random>
#include <vector>
#include <optional>
namespace angel
{

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
    for ( auto i = 0; i < tt.num_vars(); ++i )
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
}; 

} // namespace angel
