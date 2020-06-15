#include "experiments.hpp"
#include <angel/angel.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

template<class Exp>
void all_k_inputs_functions( Exp&& exp, uint32_t num_vars )
{

  using network_type = tweedledum::netlist<tweedledum::mcmt_gate>;
  network_type network;

  kitty::dynamic_truth_table tt( num_vars );
  /* TODO: implementation does not work for 0 */
  kitty::next_inplace( tt );
  auto i = 1u;
  while ( !kitty::is_const0( tt ) && kitty::count_ones(tt) != (pow(2, num_vars)) )
  {
    std::string tt_str = kitty::to_binary( tt );
    std::reverse(tt_str.begin(), tt_str.end());
    angel::qsp_add_statistics stats;
    angel::qsp_add<network_type>( network, tt_str, stats );
    //stats.report();
    exp( fmt::format( "func {}", i ), stats.nodes, stats.cnots, stats.sqgs , stats.MC_gates );
    //if(i == 100)
      //break;
    i++;
    kitty::next_inplace( tt );
  }
}

int main()
{
  // experiments::experiment<std::string, uint64_t, uint64_t, uint64_t,
  //                         uint64_t>
  //     exp( "qsp_funcs", "benchmarks", "Nodes", "CNOTs", "sqgs" , "MC_gates" );
  // auto num_vars = 4u;
  // all_k_inputs_functions( exp, num_vars );
  // exp.save();
  // exp.table();

  using network_type = tweedledum::netlist<tweedledum::mcmt_gate>;
  network_type network;
  std::string tt_str = "1101111100010100";
  //std::reverse(tt_str.begin(), tt_str.end());
  angel::qsp_add_statistics stats;
  angel::qsp_add<network_type>( network, tt_str, stats );
  stats.report();

  return 0;
}