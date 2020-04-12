#include <catch.hpp>

#include <angel/dependency_analysis/common.hpp>
#include <angel/dependency_analysis/esop_based_dependency_analysis.hpp>

#include <kitty/kitty.hpp>

#include <fmt/format.h>
#include <iostream>

TEST_CASE( "extract dependencies as ESOP cover" , "[esop_based_dependency_analysis]" )
{
  kitty::dynamic_truth_table tt{4u};
  kitty::create_from_binary_string(tt, "1000000000000001");

  {
    angel::esop_deps_analysis_params ps;
    angel::esop_deps_analysis_stats st;
    auto const result = angel::compute_dependencies<angel::esop_deps_analysis>( tt, ps, st );

    CHECK( result.dependencies.size() == 3u );
    for ( auto i = 0u; i < result.dependencies.size(); ++i )
    {
      switch( i )
      {
      case 0:
        CHECK( result.dependencies.at( 0 ) == std::vector<std::vector<uint32_t>>{{2u}} );
        break;
      case 1:
        CHECK( result.dependencies.at( 1 ) == std::vector<std::vector<uint32_t>>{{4u}} );
        break;
      case 2:
        CHECK( result.dependencies.at( 2 ) == std::vector<std::vector<uint32_t>>{{6u}} );
        break;
      default:
        /* no dependencies not listed */
        CHECK( false );
        break;
      }
    }
  }
}
