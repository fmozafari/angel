#include <angel/angel.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

#include "experiments.hpp"

template<class Exp>
void run_experiments( Exp&& exp, std::vector<std::string> const& benchmarks, std::string const& name, angel::function_extractor_params ps = {} )
{
  angel::function_extractor extractor{ps};

  angel::qsp_general_stats stats_baseline;
  angel::qsp_general_stats stats_default_order;
  angel::qsp_general_stats stats_random_reorder;
  angel::qsp_general_stats stats_deps_reorder;

  for ( const auto &benchmark : benchmarks )
  {
    fmt::print( "[i] processing {}\n", benchmark );
    if ( !extractor.parse( experiments::benchmark_path( benchmark ) ) )
      continue;

    extractor.run( [&]( kitty::dynamic_truth_table const &tt ){
        {
          /* state preparation without dependency analysis or reordering (baseline) */
          tweedledum::netlist<tweedledum::mcmt_gate> ntk;
          angel::NoDeps deps_alg;
          angel::NoReordering orders;
          
          //angel::qsp_tt_general( ntk, deps_alg, orders, tt, stats_baseline );
        }

        {
          /* state preparation with dependency analysis but no reordering */
          tweedledum::netlist<tweedledum::mcmt_gate> ntk;
          //angel::ResubSynthesisDeps deps_alg;
          angel::NoReordering orders;
          angel::qsp_tt_general<decltype( ntk ), angel::dependency_analysis_impl, decltype( orders )>(ntk, orders, tt, stats_default_order);
        }

        {
          /* state preparation with dependency analysis and random reordering */
          tweedledum::netlist<tweedledum::mcmt_gate> ntk;
          //angel::ResubSynthesisDeps deps_alg;
          angel::RandomReordering orders(5);//(ps.num_vars * ps.num_vars);
          angel::qsp_tt_general<decltype( ntk ), angel::dependency_analysis_impl, decltype( orders )>(ntk, orders, tt, stats_random_reorder);
        }

        {
          /* state preparation with dependency analysis and dependency-considered reordering */
          tweedledum::netlist<tweedledum::mcmt_gate> ntk;
          //angel::ResubSynthesisDeps deps_alg;
          angel::ConsideringDepsReordering orders{5};
          angel::qsp_tt_general<decltype( ntk ), angel::dependency_analysis_impl, decltype( orders )>(ntk, orders, tt, stats_deps_reorder);
        }
      });
  }

  exp( name, stats_baseline.total_bench,
       stats_baseline.total_cnots, angel::to_seconds( stats_baseline.total_time ),
       stats_default_order.total_cnots, angel::to_seconds( stats_default_order.total_time ),
       stats_random_reorder.total_cnots, angel::to_seconds( stats_random_reorder.total_time ),
       stats_deps_reorder.total_cnots, angel::to_seconds( stats_deps_reorder.total_time ) );
}

int main()
{
  experiments::experiment<std::string, uint32_t, uint32_t, double, uint32_t, double, uint32_t, double, uint32_t, double>
    exp( "qsp_cuts", "benchmarks", "#functions", "base: #cnots", "base: time", "no: #cnots", "no: time", "rnd: #cnots", "rnd: time", "dep: #cnots", "dep: time" );

  for ( auto i = 4u; i < 7u; ++i )
  {
    fmt::print( "[i] run experiments for {}-input cut functions\n", i );
    run_experiments( exp, experiments::epfl_benchmarks( ~experiments::epfl::hyp ), fmt::format( "EPFL benchmarks {}", i ),
                     {.num_vars = i} );
    run_experiments( exp, experiments::iscas_benchmarks(), fmt::format( "ISCAS benchmarks {}", i ),
                     {.num_vars = i} );
  }

  exp.save();
  exp.table();

  return 0;
}
