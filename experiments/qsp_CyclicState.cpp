#include <angel/angel.hpp>
#include <bitset>
#include <cstdint>
#include <kitty/kitty.hpp>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/IR/Instruction.h>

#include "experiments.hpp"

template<class Exp>
void cyclic_state( Exp&& exp, uint32_t num_vars, uint32_t num_ones )
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

  uint32_t mask = 0;
  for ( auto i = 0u; i < num_vars; i++ )
    mask |= ( 1 << i );
  uint32_t idx_base = 0;
  for ( auto i = 0u; i < num_ones; i++ )
    idx_base |= ( 1 << i );
  for ( auto i = 0u; i < num_vars; i++ )
  {
    auto idx_left = idx_base << i;
    auto idx_rotate = idx_base >> ( num_vars - i );
    uint32_t idx = ( idx_left | idx_rotate ) & mask;
    kitty::set_bit( tt, idx );
  }

  //kitty::print_binary( tt );
  //std::cout << std::endl;

  angel::qsp_deps<decltype( ntk ), decltype( esop ), decltype( random )>( ntk, esop, random, tt, qsp0_ps, qsp0_st );

  exp( fmt::format( "({},{})-cyclic state", num_vars, num_ones ), qsp0_st.num_functions, qsp0_st.num_unique_functions,
       qsp0_st.num_cnots, qsp0_st.num_sqgs, angel::to_seconds( qsp0_st.time_total ) );
}

int main()
{
  constexpr int32_t const min_num_variables = 9;
  constexpr int32_t const max_num_variables = 12;

  experiments::experiment<std::string, uint64_t, uint64_t,
                          uint32_t, uint32_t, double>
      exp( "uqsp", "benchmarks", "total func", "unqique func",
           "esop-RR: cnots", "esop-RR: sqgs", "esop-RR: time" );

  for ( auto n = min_num_variables; n < max_num_variables; n++ )
    for ( auto k = 1; k < n; k++)
      cyclic_state( exp, n, k );
  
  exp.save();
  exp.table();

  return 0;
}
