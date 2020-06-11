#include <catch.hpp>

#include <angel/dependency_analysis/common.hpp>
#include <angel/dependency_analysis/pattern_based_dependency_analysis.hpp>
#include <kitty/kitty.hpp>
#include <fmt/format.h>
#include <iostream>

TEST_CASE( "extract dependencies using pattern based dependency analysis" , "[pattern_based_dependency_analysis]" )
{
  kitty::dynamic_truth_table tt{4u};
  kitty::create_from_binary_string(tt, "000000100010110"); /* W(4) state that include one ~XOR deps for last line  */

  {
    angel::pattern_deps_analysis_params ps;
    angel::pattern_deps_analysis_stats st;
    auto const result = angel::compute_dependencies<angel::pattern_deps_analysis>( tt, ps, st );

    CHECK( result.dependencies.size() == 1u );
    for(auto d : result.dependencies)
    {
      CHECK( d.second.first == angel::dependency_analysis_types::pattern_kind::XNOR);
      CHECK( d.second.second.size() == 3u);
    }
    
  }
}
