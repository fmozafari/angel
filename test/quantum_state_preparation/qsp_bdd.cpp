#include <angel/quantum_state_preparation/qsp_bdd.hpp>
#include <catch.hpp>

#include <kitty/constructors.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

TEST_CASE( "Prepare GHZ(3) state with qsp_bdd method", "[qsp_bdd]" )
{
  tweedledum::netlist<tweedledum::mcmt_gate> ntk;

  std::string tt_str = "10000001";

  angel::qsp_bdd_statistics stats;
  angel::qsp_bdd<decltype( ntk )>( ntk, tt_str, stats );

  CHECK( stats.cnots == 5u );
  CHECK( stats.nodes == 5u );
  CHECK( stats.MC_gates == 2u );
  CHECK( stats.sqgs == 5u );
}

TEST_CASE( "Prepare a random state with 4 qubits with qsp_bdd method", "[qsp_bdd]" )
{
  tweedledum::netlist<tweedledum::mcmt_gate> ntk;

  std::string tt_str = "1111000000000011"; /* left most character show LSB */

  angel::qsp_bdd_statistics stats;
  angel::qsp_bdd<decltype( ntk )>( ntk, tt_str, stats );

  CHECK( stats.cnots == 13u );
  CHECK( stats.nodes == 4u );
  CHECK( stats.MC_gates == 5u );
  CHECK( stats.sqgs == 13u );
}
