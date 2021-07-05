#include <angel/angel.hpp>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/IR/Instruction.h>
#include <tweedledum/Utils/Visualization/string_utf8.h>


int main()
{
  tweedledum::Circuit network;
  std::string tt_str = "0110100010000000";
  angel::qsp_bdd_statistics stats;
  angel::qsp_bdd<decltype(network)>( network, tt_str, stats );
  stats.report();
  tweedledum::print(network);

  return 0;
}
