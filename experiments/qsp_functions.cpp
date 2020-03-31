#include <angel/angel.hpp>
#include <kitty/kitty.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

#include "experiments.hpp"

int main()
{
  constexpr int32_t const min_num_variables = 4;
  constexpr int32_t const max_num_variables = 8;

  experiments::experiment<std::string, uint32_t, uint32_t, double, uint32_t, double, uint32_t, double, uint32_t, double>
    exp( "qsp_cuts", "benchmarks", "#functions", "base: #cnots", "base: time", "no: #cnots", "no: time", "rnd: #cnots", "rnd: time", "dep: #cnots", "dep: time" );

  for ( auto num_vars = min_num_variables; num_vars <= max_num_variables; ++num_vars )
  {
    angel::qsp_general_stats stats_baseline;
    angel::qsp_general_stats stats_default_order;
    angel::qsp_general_stats stats_random_reorder;
    angel::qsp_general_stats stats_deps_reorder;

    angel::deps_operation_stats op_stats_baseline;
    angel::deps_operation_stats op_stats_default_order;
    angel::deps_operation_stats op_stats_random_reorder;
    angel::deps_operation_stats op_stats_deps_reorder;  

    kitty::dynamic_truth_table tt( num_vars );
    for ( auto i = 0; i < num_vars; ++i )
    {
      kitty::create_equals( tt, i );
      
      {
        /* state preparation without dependency analysis or reordering (baseline) */
        tweedledum::netlist<tweedledum::mcmt_gate> ntk;
        angel::NoDeps deps_alg;
        angel::NoReordering orders;
        angel::qsp_tt_general( ntk, deps_alg, orders, tt, stats_baseline, op_stats_baseline );
      }

      {
        /* state preparation with dependency analysis but no reordering */
        tweedledum::netlist<tweedledum::mcmt_gate> ntk;
        angel::ResubSynthesisDeps deps_alg;
        angel::NoReordering orders;
        angel::qsp_tt_general( ntk, deps_alg, orders, tt, stats_default_order, op_stats_default_order );
      }

      {
        /* state preparation with dependency analysis and random reordering */
        tweedledum::netlist<tweedledum::mcmt_gate> ntk;
        angel::ResubSynthesisDeps deps_alg;
        angel::RandomReordering orders(num_vars*num_vars);
        angel::qsp_tt_general( ntk, deps_alg, orders, tt, stats_random_reorder, op_stats_random_reorder );
      }

      {
        /* state preparation with dependency analysis and dependency-considered reordering */
        tweedledum::netlist<tweedledum::mcmt_gate> ntk;
        angel::ResubSynthesisDeps deps_alg;
        angel::ConsideringDepsReordering orders{5};
        angel::qsp_tt_general( ntk, deps_alg, orders, tt, stats_deps_reorder, op_stats_deps_reorder );
      }
    }

    exp( fmt::format( "{:2d}-equals-k benchmarks", num_vars ), stats_default_order.total_bench,
       stats_baseline.total_cnots, angel::to_seconds( stats_baseline.total_time ),
       stats_default_order.total_cnots, angel::to_seconds( stats_default_order.total_time ),
       stats_random_reorder.total_cnots, angel::to_seconds( stats_random_reorder.total_time ),
       stats_deps_reorder.total_cnots, angel::to_seconds( stats_deps_reorder.total_time ) );
  }

  exp.save();
  exp.table();

  return 0;
}
