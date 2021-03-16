#include <angel/angel.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>


int main()
{
  using network_type = tweedledum::netlist<tweedledum::mcmt_gate>;
  network_type network;
  std::string tt_str = "1111000000000011";
  angel::qsp_bdd_statistics stats;
  angel::qsp_bdd<network_type>( network, tt_str, stats );
  stats.report();

  return 0;
}
