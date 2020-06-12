#include <angel/angel.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include "experiments.hpp"

int main()
{ 
  using network_type = tweedledum::netlist<tweedledum::mcmt_gate>;
  network_type network;
  std::string tt_str = "10000001";
  angel::qsp_add_statistics stats;

  angel::qsp_add<network_type> ( network, tt_str, stats);
  stats.report();

  return 0;
}