/* author: Fereshte */
#include <catch.hpp>
#include <angel/quantum_state_preparation/qsp_tt_general.hpp>
#include <angel/reordering/no_reordering.hpp>
#include <angel/dependency_analysis/esop_based_dependency_analysis.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <kitty/constructors.hpp>

TEST_CASE( "Synthesize ESOPs", "[esop]" )
{
  std::vector<std::string> fs = {
    "69696996696969966969699669969696",
    "69969669699696693cc3c33c69969669",
    "c396963cc396963cc396963ccc999933",
    "695aa569695aa569695aa5696655aa66",
    "963cc396963cc3969933cc99963cc396",
    "a596965aa596965aa596965a556666aa",
    "a95656a9a95656a9a95656a9956a6a95",
    "6a95956a6a95956a6a95956a56a9a956",
    "5959599a5959599a5959599a599a9a9a",
    "556666aa5a6969a55a6969a55a6969a5",
  };
  
  angel::no_reordering no_reorder;
  
  typename angel::esop_deps_analysis::parameter_type esop_ps;
  typename angel::esop_deps_analysis::statistics_type esop_st;
  angel::esop_deps_analysis esop( esop_ps, esop_st );

  angel::state_preparation_parameters ps;
  angel::state_preparation_statistics st;  
  angel::state_preparation prep( esop, no_reorder, ps, st );

  kitty::dynamic_truth_table tt{7};
  for ( const auto& f : fs )
  {
    create_from_hex_string( tt, f );
    prep( tt );
  }
}
