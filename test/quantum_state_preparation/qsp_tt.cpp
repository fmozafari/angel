/* author: Fereshte */
#include <catch.hpp>
// #include <angel/quantum_state_preparation/qsp_tt.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <kitty/constructors.hpp>

// using namespace angel;

TEST_CASE("Prepare GHZ(3) state with qsp_tt method", "[qsp_tt]")
{
	tweedledum::netlist<tweedledum::mcmt_gate> network;
        
	kitty::dynamic_truth_table tt(3);
	kitty::create_from_binary_string(tt, "10000001");

	// qsp_tt_statistics stats;
	// qsp_tt(network, tt, stats);
        // 
	// CHECK(stats.total_cnots == 5u);
}
