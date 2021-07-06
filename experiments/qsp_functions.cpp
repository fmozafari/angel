#include <angel/angel.hpp>
#include <kitty/kitty.hpp>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/IR/Instruction.h>

#include "experiments.hpp"

template<class Exp>
void k_equal_function( Exp&& exp, uint32_t num_vars )
{
  tweedledum::Circuit ntk;
  /* dependency analysis strategies */
  typename angel::esop_deps_analysis::parameter_type esop_ps;
  typename angel::esop_deps_analysis::statistics_type esop_st;
  angel::esop_deps_analysis esop( esop_ps, esop_st );

  /* reordering strategies */
  angel::random_reordering random( 0xcafeaffe, num_vars > 1u ? ( num_vars * num_vars ) : 1u );

  /* prepare state preparation algorithms */
  angel::state_preparation_parameters qsp0_ps;
  angel::state_preparation_statistics qsp0_st;
  
  for ( auto i = 0u; i < num_vars; ++i )
  {
    kitty::dynamic_truth_table tt( num_vars );
    kitty::create_equals( tt, i );

    angel::qsp_deps<decltype( ntk ), decltype( esop ), decltype( random )> ( ntk, esop, random, tt, qsp0_ps, qsp0_st );
  }

  exp( fmt::format( "{:2d}-equals-k functions", num_vars ), qsp0_st.num_functions, qsp0_st.num_unique_functions,
       qsp0_st.num_cnots, qsp0_st.num_sqgs, angel::to_seconds( qsp0_st.time_total ) );
}

template<class Exp>
void ghz_state( Exp&& exp, uint32_t num_vars )
{
  tweedledum::Circuit ntk;
  /* dependency analysis strategies */
  typename angel::esop_deps_analysis::parameter_type esop_ps;
  typename angel::esop_deps_analysis::statistics_type esop_st;
  angel::esop_deps_analysis esop( esop_ps, esop_st );

  typename angel::no_deps_analysis::parameter_type no_dependencies_ps;
  typename angel::no_deps_analysis::statistics_type no_dependencies_st;
  angel::no_deps_analysis no_deps( no_dependencies_ps, no_dependencies_st );

  /* reordering strategies */
  angel::random_reordering random( 0xcafeaffe, num_vars > 1u ? ( num_vars * num_vars ) : 1u );
  angel::no_reordering no_reorder;

  /* prepare state preparation algorithms */
  angel::state_preparation_parameters qsp0_ps;
  angel::state_preparation_statistics qsp0_st;
  

  kitty::dynamic_truth_table tt( num_vars );

  kitty::set_bit( tt, 0 );
  kitty::set_bit( tt, pow( 2, num_vars ) - 1 );

  angel::qsp_deps<decltype( ntk ), decltype( no_deps ), decltype( no_reorder )> ( ntk, no_deps, no_reorder, tt, qsp0_ps, qsp0_st );

  exp( fmt::format( "{:2d}-ghz state", num_vars ), qsp0_st.num_functions, qsp0_st.num_unique_functions,
       qsp0_st.num_cnots, qsp0_st.num_sqgs, angel::to_seconds( qsp0_st.time_total ) );
}

template<class Exp>
void w_state( Exp&& exp, uint32_t num_vars )
{
  tweedledum::Circuit ntk;
  /* dependency analysis strategies */
  typename angel::esop_deps_analysis::parameter_type esop_ps;
  typename angel::esop_deps_analysis::statistics_type esop_st;
  angel::esop_deps_analysis esop( esop_ps, esop_st );

  typename angel::no_deps_analysis::parameter_type no_dependencies_ps;
  typename angel::no_deps_analysis::statistics_type no_dependencies_st;
  angel::no_deps_analysis no_deps( no_dependencies_ps, no_dependencies_st );

  /* reordering strategies */
  angel::random_reordering random( 0xcafeaffe, num_vars > 1u ? ( num_vars * num_vars ) : 1u );
  angel::no_reordering no_reorder;

  /* prepare state preparation algorithms */
  angel::state_preparation_parameters qsp0_ps;
  angel::state_preparation_statistics qsp0_st;

  kitty::dynamic_truth_table tt( num_vars );
  for ( auto i = 0u; i < num_vars; i++ )
  {
    kitty::set_bit( tt, 1 << i );
  }

  angel::qsp_deps<decltype( ntk ), decltype( no_deps ), decltype( no_reorder )> ( ntk, no_deps, no_reorder, tt, qsp0_ps, qsp0_st );

  exp( fmt::format( "{:2d}-w state", num_vars ), qsp0_st.num_functions, qsp0_st.num_unique_functions,
       qsp0_st.num_cnots, qsp0_st.num_sqgs, angel::to_seconds( qsp0_st.time_total ) );
}

template<class Exp>
void QBA_state( Exp&& exp, uint32_t num_vars )
{
  tweedledum::Circuit ntk;
  /* dependency analysis strategies */
  typename angel::esop_deps_analysis::parameter_type esop_ps;
  typename angel::esop_deps_analysis::statistics_type esop_st;
  angel::esop_deps_analysis esop( esop_ps, esop_st );

  typename angel::no_deps_analysis::parameter_type no_dependencies_ps;
  typename angel::no_deps_analysis::statistics_type no_dependencies_st;
  angel::no_deps_analysis no_deps( no_dependencies_ps, no_dependencies_st );

  /* reordering strategies */
  angel::random_reordering random( 0xcafeaffe, num_vars > 1u ? ( num_vars * num_vars ) : 1u );
  angel::no_reordering no_reorder;

  /* prepare state preparation algorithms */
  angel::state_preparation_parameters qsp0_ps;
  angel::state_preparation_statistics qsp0_st;
  

  kitty::dynamic_truth_table tt( num_vars );
  for ( auto i = 0; i < pow( num_vars, 3 ); i++ )
  {
    kitty::set_bit( tt, i );
  }

  angel::qsp_deps<decltype( ntk ), decltype( esop ), decltype( random )> ( ntk, esop, random, tt, qsp0_ps, qsp0_st );
  
  exp( fmt::format( "{:2d}-QBA state", num_vars ), qsp0_st.num_functions, qsp0_st.num_unique_functions,
       qsp0_st.num_cnots, qsp0_st.num_sqgs, angel::to_seconds( qsp0_st.time_total ) );
}

int main()
{
  constexpr int32_t const min_num_variables = 10;
  constexpr int32_t const max_num_variables = 12;

  experiments::experiment<std::string, uint64_t, uint64_t,
                          uint32_t, uint32_t, double>
      exp( "uqsp", "benchmarks", "total func", "unqique func",
           "esop-RR: cnots", "esop-RR: sqgs", "esop-RR: time" );

  for ( auto num_vars = min_num_variables; num_vars < max_num_variables; num_vars += 2 )
  {
    k_equal_function( exp, num_vars );
    ghz_state( exp, num_vars );
    w_state( exp, num_vars );
    QBA_state( exp, num_vars ); /* Works for variables more than 9 */
  }
  exp.save();
  exp.table();

  return 0;
}
