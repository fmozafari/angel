#include <angel/angel.hpp>
#include <kitty/kitty.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

#include "experiments.hpp"

template<class Exp>
void k_equal_function(Exp&& exp, uint32_t num_vars)
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

  exp( fmt::format( "{:2d}-equals-k functions", num_vars ), qsp0_st.num_functions, qsp0_st.num_unique_functions,
      qsp0_st.num_cnots, angel::to_seconds( qsp0_st.time_total ) );
}

template<class Exp>
void ghz_state(Exp&& exp, uint32_t num_vars)
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
  
  kitty::set_bit(tt,0);
  kitty::set_bit(tt, pow(2,num_vars)-1);
  //kitty::print_binary(tt);
  p0(tt);

  exp( fmt::format( "{:2d}-ghz state", num_vars ), qsp0_st.num_functions, qsp0_st.num_unique_functions,
      qsp0_st.num_cnots, angel::to_seconds( qsp0_st.time_total ) );
}

template<class Exp>
void w_state(Exp&& exp, uint32_t num_vars)
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
  for(auto i=0; i<num_vars; i++)
  {
    kitty::set_bit(tt,1<<i);
  }
  
  kitty::print_binary(tt);
  std::cout<<std::endl;

  p0(tt);

  exp( fmt::format( "{:2d}-w state", num_vars ), qsp0_st.num_functions, qsp0_st.num_unique_functions,
      qsp0_st.num_cnots, angel::to_seconds( qsp0_st.time_total ) );
}

int main()
{
  constexpr int32_t const min_num_variables = 4;
  constexpr int32_t const max_num_variables = 7;

  experiments::experiment<std::string, uint64_t, uint64_t,
                          uint32_t, double>
  exp( "qsp_cuts", "benchmarks", "total func", "unqique func",
        "esop-RR: cnots", "esop-RR: time" );

  

  for ( auto num_vars = min_num_variables; num_vars < max_num_variables; num_vars+=2 )
  {
    //k_equal_function(exp, num_vars);
    //ghz_state(exp, num_vars);
    w_state(exp, num_vars);
  }
  exp.save();
  exp.table();

  return 0;
}
