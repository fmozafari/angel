#include <angel/angel.hpp>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/IR/Instruction.h>
#include <tweedledum/Utils/Visualization/string_utf8.h>


int main()
{
  using network_type = tweedledum::Circuit;
  network_type network;
  std::string tt_str = "0110100010000000";
  angel::qsp_bdd_statistics stats;
  angel::qsp_bdd<network_type>( network, tt_str, stats );
  stats.report();
  tweedledum::print(network);

  return 0;
}
