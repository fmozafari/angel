#include <angel/utils/function_extractor.hpp>

#include <angel/quantum_state_preparation/qsp_tt_general.hpp>
#include <angel/reordering/random_reordering.hpp>
#include <angel/reordering/all_reordering.hpp>
#include <angel/reordering/no_reordering.hpp>
#include <angel/reordering/considering_deps_reordering.hpp>
#include <angel/dependency_analysis/resub_synthesis.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <angel/utils/stopwatch.hpp>

#include "experiments.hpp"

int main()
{
  angel::function_extractor_params ps;
  ps.num_vars = 4;
  ps.exact_size = true;
  angel::function_extractor extractor{ps};

  experiments::experiment<std::string, uint32_t, uint32_t, double, uint32_t, double, uint32_t, double>
    exp( "qsp_cuts", "benchmarks", "#functions", "base: #cnots", "base: time", "rnd: #cnots", "rnd: time", "dep: #cnots", "dep: time" );

  angel::qsp_general_stats stats_baseline;
  angel::qsp_general_stats stats_random_reorder;
  angel::qsp_general_stats stats_deps_reorder;

  for ( const auto &benchmark : experiments::epfl_benchmarks( ~experiments::hyp ) )
  {
    fmt::print( "[i] processing {}\n", benchmark );
    if ( !extractor.parse( experiments::benchmark_path( benchmark ) ) )
      continue;

    extractor.run( [&]( kitty::dynamic_truth_table const &tt ){
        {
          /* state preparation without reordering (baseline) */
          tweedledum::netlist<tweedledum::mcmt_gate> ntk;
          angel::NoDeps deps_alg;
          angel::NoReordering orders;
          angel::qsp_tt_general( ntk, deps_alg, orders, tt, stats_baseline );
        }

        {
          /* state preparation with random reordering */
          tweedledum::netlist<tweedledum::mcmt_gate> ntk;
          angel::ResubSynthesisDeps deps_alg;
          angel::RandomReordering orders{5};
          angel::qsp_tt_general( ntk, deps_alg, orders, tt, stats_random_reorder );
        }

        {
          /* state preparation with dependency-considered reordering */
          tweedledum::netlist<tweedledum::mcmt_gate> ntk;
          angel::ResubSynthesisDeps deps_alg;
          angel::ConsideringDepsReordering orders{5};
          angel::qsp_tt_general( ntk, deps_alg, orders, tt, stats_deps_reorder );
        }
      });
  }

  exp( "EPFL benchmarks", stats_baseline.total_bench,
       stats_baseline.total_cnots, angel::to_seconds( stats_baseline.total_time ),
       stats_random_reorder.total_cnots, angel::to_seconds( stats_random_reorder.total_time ),
       stats_deps_reorder.total_cnots, angel::to_seconds( stats_deps_reorder.total_time ) );

  exp.save();
  exp.table();

  return 0;
}
