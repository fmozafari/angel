#include <angel/angel.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include "experiments.hpp"

template<class Exp>
void run_experiments( Exp&& exp, std::vector<std::string> const& benchmarks, std::string const& name, angel::function_extractor_params extract_ps = {} )
{
  using network_type = tweedledum::netlist<tweedledum::mcmt_gate>;

  angel::function_extractor extractor{extract_ps};

  /* dependency analysis strategies */
  typename angel::no_deps_analysis::parameter_type no_dependencies_ps;
  typename angel::no_deps_analysis::statistics_type no_dependencies_st;
  angel::no_deps_analysis no_deps( no_dependencies_ps, no_dependencies_st );

  typename angel::pattern_deps_analysis::parameter_type pattern_ps;
  typename angel::pattern_deps_analysis::statistics_type pattern_st;
  angel::pattern_deps_analysis pattern( pattern_ps, pattern_st );

  typename angel::esop_deps_analysis::parameter_type esop_ps;
  typename angel::esop_deps_analysis::statistics_type esop_st;
  angel::esop_deps_analysis esop( esop_ps, esop_st );
    
  /* reordering strategies */
  angel::no_reordering no_reorder;
  angel::exhaustive_reordering all_orders;
  angel::random_reordering random( 0xcafeaffe, extract_ps.num_vars > 1u ? ( extract_ps.num_vars * extract_ps.num_vars ) : 1u );
  angel::greedy_reordering greedy;

  /* prepare state preparation algorithms */
  angel::state_preparation_parameters qsp0_ps;
  angel::state_preparation_statistics qsp0_st;
  angel::state_preparation<decltype( no_deps ), decltype( no_reorder )> p0( no_deps, no_reorder, qsp0_ps, qsp0_st );

  angel::state_preparation_parameters qsp1_ps;
  angel::state_preparation_statistics qsp1_st;
  angel::state_preparation<decltype( pattern ), decltype( no_reorder )> p1( pattern, no_reorder, qsp1_ps, qsp1_st );

  angel::state_preparation_parameters qsp2_ps;
  angel::state_preparation_statistics qsp2_st;
  angel::state_preparation<decltype( esop ), decltype( no_reorder )> p2( esop, no_reorder, qsp2_ps, qsp2_st );

  angel::state_preparation_parameters qsp3_ps;
  angel::state_preparation_statistics qsp3_st;
  angel::state_preparation<decltype( no_deps ), decltype( random )> p3( no_deps, random, qsp3_ps, qsp3_st );

  angel::state_preparation_parameters qsp4_ps;
  angel::state_preparation_statistics qsp4_st;
  angel::state_preparation<decltype( pattern ), decltype( random )> p4( pattern, random, qsp4_ps, qsp4_st );

  angel::state_preparation_parameters qsp5_ps;
  angel::state_preparation_statistics qsp5_st;
  angel::state_preparation<decltype( esop ), decltype( random )> p5( esop, random, qsp5_ps, qsp5_st );

  angel::state_preparation_parameters qsp6_ps;
  angel::state_preparation_statistics qsp6_st;
  angel::state_preparation<decltype( no_deps ), decltype( greedy )> p6( no_deps, greedy, qsp6_ps, qsp6_st );

  angel::state_preparation_parameters qsp7_ps;
  angel::state_preparation_statistics qsp7_st;
  angel::state_preparation<decltype( pattern ), decltype( greedy )> p7( pattern, greedy, qsp7_ps, qsp7_st );

  angel::state_preparation_parameters qsp8_ps;
  angel::state_preparation_statistics qsp8_st;
  angel::state_preparation<decltype( esop ), decltype( greedy )> p8( esop, greedy, qsp8_ps, qsp8_st );

  angel::state_preparation_parameters qsp9_ps;
  angel::state_preparation_statistics qsp9_st;
  angel::state_preparation<decltype( esop ), decltype( all_orders )> p9( esop, all_orders, qsp9_ps, qsp9_st );
  
  for ( const auto& benchmark : benchmarks )
  {
    fmt::print( "[i] processing {}\n", benchmark );
    if ( !extractor.parse( experiments::benchmark_path( benchmark ) ) )
    {
      fmt::print( "[e] could not parse file {}\n", experiments::benchmark_path( benchmark ) );
      continue;
    }

    extractor.run( [&]( kitty::dynamic_truth_table const& tt ){
        p0( tt ); /* baseline */
        p1( tt ); /* patterns + no reordering */
        p2( tt ); /* ESOPs + no reordering */

        p3( tt ); /* no dependencies + random reordering */
        p4( tt ); /* patterns + random reordering */
        p5( tt ); /* ESOPs + random reordering */

        p6( tt ); /* no dependencies + greedy reordering */
        p7( tt ); /* patterns + greedy reordering */
        p8( tt ); /* ESOPs + greedy reordering */

        //p9(tt); /* ESOPs + all orders */

        /* ensure that baseline has the highest costs */
        if ( qsp0_st.num_cnots < qsp1_st.num_cnots ||
             qsp0_st.num_cnots < qsp2_st.num_cnots ||
             qsp0_st.num_cnots < qsp3_st.num_cnots ||
             qsp0_st.num_cnots < qsp4_st.num_cnots ||
             qsp0_st.num_cnots < qsp5_st.num_cnots ||
             qsp0_st.num_cnots < qsp6_st.num_cnots ||
             qsp0_st.num_cnots < qsp7_st.num_cnots ||
             qsp0_st.num_cnots < qsp8_st.num_cnots )
        {
          std::abort();
        }
      });
  }

  exp( name, qsp0_st.num_functions, qsp0_st.num_unique_functions,
       qsp0_st.num_cnots, angel::to_seconds( qsp0_st.time_total ),
       qsp1_st.num_cnots, angel::to_seconds( qsp1_st.time_total ),
       qsp2_st.num_cnots, angel::to_seconds( qsp2_st.time_total ),
       
       qsp3_st.num_cnots, angel::to_seconds( qsp3_st.time_total ),
       qsp4_st.num_cnots, angel::to_seconds( qsp4_st.time_total ),
       qsp5_st.num_cnots, angel::to_seconds( qsp5_st.time_total ),

       qsp6_st.num_cnots, angel::to_seconds( qsp6_st.time_total ),
       qsp7_st.num_cnots, angel::to_seconds( qsp7_st.time_total ),
       qsp8_st.num_cnots, angel::to_seconds( qsp8_st.time_total ),
       qsp9_st.num_cnots, angel::to_seconds( qsp9_st.time_total )
  );
}

int main()
{  
  experiments::experiment<std::string, uint64_t, uint64_t,
                          uint64_t, double, uint64_t, double, uint64_t, double,
                          uint64_t, double, uint64_t, double, uint64_t, double,
                          uint64_t, double, uint64_t, double, uint64_t, double, uint32_t, double>
    exp( "qsp_cuts", "benchmarks", "total func", "unqique func",
         "cnot qsp0", "time qsp0", "cnot qsp1", "time qsp1", "cnot qsp2", "time qsp2",
         "cnot qsp3", "time qsp3", "cnot qsp4", "time qsp4", "cnot qsp5", "time qsp5",
         "cnot qsp6", "time qsp6", "cnot qsp7", "time qsp7", "cnot qsp8", "time qsp8", "cnot qsp9", "time qsp9" );
  
  for ( auto i = 6u; i < 8u; ++i )
  {
    fmt::print( "[i] run experiments for {}-input cut functions\n", i );
    run_experiments( exp, experiments::epfl_benchmarks(), fmt::format( "EPFL benchmarks {}", i ),
                     {.num_vars = i} );
    run_experiments( exp, experiments::iscas_benchmarks(), fmt::format( "ISCAS benchmarks {}", i ),
                     {.num_vars = i} );
  }

  exp.save();
  exp.table();

  return 0;
}
