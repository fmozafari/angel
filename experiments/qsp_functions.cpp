#include <angel/angel.hpp>
#include <kitty/kitty.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

#include "experiments.hpp"

int main()
{
  constexpr int32_t const min_num_variables = 4;
  constexpr int32_t const max_num_variables = 8;

  experiments::experiment<std::string, uint32_t, uint32_t, double, uint32_t, double, uint32_t, double, uint32_t, double, 
  uint32_t, double, uint32_t, double, uint32_t, double>
    exp( "qsp_cuts", "benchmarks", "#functions", "base: #cnots", "base: time", "no-pattern: #cnots", "no-pattern: time", 
    "no-esop: #cnots", "no-esop: time", "rnd-pattern: #cnots", "rnd-pattern: time", "rnd-esop: #cnots", "rnd-esop: time",
    "sift-pattern: #cnots", "sift-pattern: time", "sift-esop: #cnots", "sift-esop: time" );

  for ( auto num_vars = min_num_variables; num_vars <= max_num_variables; ++num_vars )
  {
    angel::qsp_general_stats stats_baseline;
    angel::qsp_general_stats stats_defaultOrder_patternDeps;
    angel::qsp_general_stats stats_defaultOrder_esopDeps;
    angel::qsp_general_stats stats_randomOrder_patternDeps;
    angel::qsp_general_stats stats_randomOrder_esopDeps;
    angel::qsp_general_stats stats_siftOrder_patternDeps;
    angel::qsp_general_stats stats_siftOrder_esopDeps;

    auto seed = 1;
    uint32_t counter = num_vars;
    while(counter!= 0)
    {
        seed *= counter;
        counter--;
    }


    kitty::dynamic_truth_table tt( num_vars );
    for ( auto i = 0; i < num_vars; ++i )
    {
      kitty::create_equals( tt, i );
      
      {
          /* state preparation without dependency analysis or reordering (baseline) */
          tweedledum::netlist<tweedledum::mcmt_gate> ntk;
          angel::NoReordering orders;
          angel::qsp_tt_general<decltype( ntk ), angel::no_deps_analysis, decltype( orders )>(ntk, orders, tt, stats_baseline);
        }


        {
          /* state preparation with pattern dependency analysis but no reordering */
          tweedledum::netlist<tweedledum::mcmt_gate> ntk;
          angel::NoReordering orders;
          angel::qsp_tt_general<decltype( ntk ), angel::pattern_deps_analysis, decltype( orders )>(ntk, orders, tt, stats_defaultOrder_patternDeps);
        }

        {
          /* state preparation with esop dependency analysis but no reordering */
          tweedledum::netlist<tweedledum::mcmt_gate> ntk;
          angel::NoReordering orders;
          angel::qsp_tt_general<decltype( ntk ), angel::esop_deps_analysis, decltype( orders )>(ntk, orders, tt, stats_defaultOrder_esopDeps);
        }

        {
          /* state preparation with pattern dependency analysis and random reordering */
          tweedledum::netlist<tweedledum::mcmt_gate> ntk;
          angel::RandomReordering orders(seed, num_vars * num_vars);
          angel::qsp_tt_general<decltype( ntk ), angel::pattern_deps_analysis, decltype( orders )>(ntk, orders, tt, stats_randomOrder_patternDeps);
        }

        {
          /* state preparation with esop dependency analysis and random reordering */
          tweedledum::netlist<tweedledum::mcmt_gate> ntk;
          angel::RandomReordering orders(seed, num_vars * num_vars);
          angel::qsp_tt_general<decltype( ntk ), angel::esop_deps_analysis, decltype( orders )>(ntk, orders, tt, stats_randomOrder_esopDeps);
        }

        {
          /* state preparation with pattern dependency analysis and sift reordering */
          tweedledum::netlist<tweedledum::mcmt_gate> ntk;
          angel::SiftReordering orders(tt);
          angel::qsp_tt_general<decltype( ntk ), angel::pattern_deps_analysis, decltype( orders )>(ntk, orders, tt, stats_siftOrder_patternDeps);
        }

        {
          /* state preparation with esop dependency analysis and random reordering */
          tweedledum::netlist<tweedledum::mcmt_gate> ntk;
          angel::SiftReordering orders(tt);
          angel::qsp_tt_general<decltype( ntk ), angel::esop_deps_analysis, decltype( orders )>(ntk, orders, tt, stats_siftOrder_esopDeps);
        }

    }

    exp( fmt::format( "{:2d}-equals-k benchmarks", num_vars ), stats_baseline.total_bench,
       stats_baseline.total_cnots, angel::to_seconds( stats_baseline.total_time ),
       stats_defaultOrder_patternDeps.total_cnots, angel::to_seconds( stats_defaultOrder_patternDeps.total_time ),
       stats_defaultOrder_esopDeps.total_cnots, angel::to_seconds( stats_defaultOrder_esopDeps.total_time ),
       stats_randomOrder_patternDeps.total_cnots, angel::to_seconds( stats_randomOrder_patternDeps.total_time ),
       stats_randomOrder_esopDeps.total_cnots, angel::to_seconds( stats_randomOrder_esopDeps.total_time ),
       stats_siftOrder_patternDeps.total_cnots, angel::to_seconds( stats_siftOrder_patternDeps.total_time ),
       stats_siftOrder_esopDeps.total_cnots, angel::to_seconds( stats_siftOrder_esopDeps.total_time ) );
  }

  exp.save();
  exp.table();

  return 0;
}
