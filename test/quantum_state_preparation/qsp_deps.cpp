/* author: Fereshte */
#include <angel/dependency_analysis/esop_based_dependency_analysis.hpp>
#include <angel/dependency_analysis/pattern_based_dependency_analysis.hpp>
#include <angel/quantum_state_preparation/qsp_deps.hpp>
#include <angel/reordering/no_reordering.hpp>
#include <angel/reordering/exhaustive_reordering.hpp>
#include <angel/reordering/random_reordering.hpp>
#include <catch.hpp>
#include <kitty/constructors.hpp>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/IR/Instruction.h>

TEST_CASE( "Prepare GHZ(3) state with qsp using esop dependency method", "[qsp_deps]" )
{
  tweedledum::Circuit network;
  kitty::dynamic_truth_table tt( 3 );
  kitty::create_from_binary_string( tt, "10000001" );
  /* reordering strategy */
  angel::no_reordering no_reorder;
  /* dependency analysis method */
  typename angel::esop_deps_analysis::parameter_type esop_ps;
  typename angel::esop_deps_analysis::statistics_type esop_st;
  angel::esop_deps_analysis esop( esop_ps, esop_st );

  angel::state_preparation_parameters qsp_ps;
  angel::state_preparation_statistics qsp_st;
  angel::qsp_deps<decltype( network ), decltype( esop ), decltype( no_reorder )> ( network, esop, no_reorder, tt, qsp_ps, qsp_st );

  CHECK( qsp_st.num_cnots == 2u );
  CHECK( qsp_st.num_sqgs == 1u );
}

TEST_CASE( "Prepare w(3) state with qsp using pattern based dependency method", "[qsp_deps]" )
{
  tweedledum::Circuit network;
  kitty::dynamic_truth_table tt( 3 );
  kitty::create_from_binary_string( tt, "00010110" ); /* left bit shows MSB */
  /* reordering strategy */
  angel::no_reordering no_reorder;
  /* dependency analysis method */
  typename angel::pattern_deps_analysis::parameter_type pattern_ps;
  typename angel::pattern_deps_analysis::statistics_type pattern_st;
  angel::pattern_deps_analysis pattern( pattern_ps, pattern_st );

  angel::state_preparation_parameters qsp_ps;
  angel::state_preparation_statistics qsp_st;
  angel::qsp_deps<decltype( network ), decltype( pattern ), decltype( no_reorder )> ( network, pattern, no_reorder, tt, qsp_ps, qsp_st );

  CHECK( qsp_st.num_cnots == 4u );
  CHECK( qsp_st.num_sqgs == 4u );
}

TEST_CASE( "Prepare " "1100" "0000" "0011" "0111" " state and utilize reordering", "[qsp_deps]" )
{
  tweedledum::Circuit network;
  uint32_t num_vars = 4;
  kitty::dynamic_truth_table tt( num_vars );
  std::string truth_table{ "1100" "0000" "0011" "0111" };
  kitty::create_from_binary_string( tt, truth_table );

  typename angel::esop_deps_analysis::parameter_type esop_ps;
  typename angel::esop_deps_analysis::statistics_type esop_st;
  angel::esop_deps_analysis esop( esop_ps, esop_st );

  /* reordering strategies */
  angel::no_reordering no_reorder;
  angel::exhaustive_reordering all_orders;
  angel::random_reordering random( 0xcafeaffe, num_vars > 1u ? ( num_vars * num_vars ) : 1u );

  angel::state_preparation_parameters qsp1_ps;
  angel::state_preparation_statistics qsp1_st;
  angel::qsp_deps<decltype( network ), decltype( esop ), decltype( no_reorder )> ( network, esop, no_reorder, tt, qsp1_ps, qsp1_st );
  
  CHECK( qsp1_st.num_cnots == 14u );
  CHECK( qsp1_st.num_sqgs == 15u );

  angel::state_preparation_parameters qsp2_ps;
  angel::state_preparation_statistics qsp2_st;
  angel::qsp_deps<decltype( network ), decltype( esop ), decltype( random )> ( network, esop, random, tt, qsp2_ps, qsp2_st );

  CHECK( qsp2_st.num_cnots == 10u );
  CHECK( qsp2_st.num_sqgs == 11u );

}
