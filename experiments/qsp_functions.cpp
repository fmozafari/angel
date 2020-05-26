#include <angel/angel.hpp>
#include <kitty/kitty.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

#include "experiments.hpp"

int main()
{
  constexpr int32_t const min_num_variables = 4;
  constexpr int32_t const max_num_variables = 10;

  experiments::experiment<std::string, uint64_t, uint64_t,
                          uint32_t, double>
  exp( "qsp_cuts", "benchmarks", "total func", "unqique func",
        "esop-RR: cnots", "esop-RR: time" );

  using network_type = tweedledum::netlist<tweedledum::mcmt_gate>;

  for ( auto num_vars = min_num_variables; num_vars <= max_num_variables; ++num_vars )
  {
    /* dependency analysis strategies */
    typename angel::esop_deps_analysis::parameter_type esop_ps;
    typename angel::esop_deps_analysis::statistics_type esop_st;
    angel::esop_deps_analysis esop( esop_ps, esop_st );

    /* reordering strategies */
    angel::random_reordering random( 0xcafeaffe, num_vars > 1u ? ( num_vars * num_vars ) : 1u );

    /* prepare state preparation algorithms */
    angel::state_preparation_parameters qsp0_ps;
    angel::state_preparation_statistics qsp0_st;
    angel::state_preparation<decltype( esop ), decltype( random )> p0( esop, random, qsp0_ps, qsp0_st );

    kitty::dynamic_truth_table tt( num_vars );
    for ( auto i = 0; i < num_vars; ++i )
    {
      kitty::create_equals( tt, i );
      p0(tt);
    }

     exp( fmt::format( "{:2d}-equals-k benchmarks", num_vars ), qsp0_st.num_functions, qsp0_st.num_unique_functions,
       qsp0_st.num_cnots, angel::to_seconds( qsp0_st.time_total ) );
  }
  exp.save();
  exp.table();

  return 0;
}
