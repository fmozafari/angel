#pragma once
//#include <angel/quantum_state_preparation/qsp_bdd.hpp>
#define _USE_MATH_DEFINES
#include <cmath> 
#include "utils.hpp"
#include <angel/utils/helper_functions.hpp>
#include <angel/utils/stopwatch.hpp>
#include <angel/quantum_circuit/create_quantum_circuit.hpp>
#include <cplusplus/cuddObj.hh>
#include <cudd/cudd.h>
#include <cudd/cuddInt.h>
#include <fstream>
#include <kitty/kitty.hpp>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include "common_bdd.hpp"

namespace angel{

struct sparse_qsp_statistics
{
  double time{ 0 };
  uint32_t cnots{ 0 };
  uint32_t sqgs{ 0 };
  uint32_t nodes{ 0 };
  uint32_t MC_gates{ 0 };
  uint32_t ancillaes{ 0 };
  void report( std::ostream& os = std::cout ) const
  {
    os << "[i] time: " << time << std::endl;
    os << "[i] nodes: " << nodes << std::endl;
    os << "[i] MC_gates: " << MC_gates << std::endl;
    os << "[i] cnots: " << cnots << std::endl;
    os << "[i] sqgs: " << sqgs << std::endl;
    os << "[i] ancillaes: " << ancillaes << std::endl;
  }
};


namespace detail{


} //namespace detail end
//**************************************************************

/**
 * \breif Sparse Quantum State Preparation using Decision Diagram
 * 
 * \tparam Network the type of generated quantum circuit
 * \param network the extracted quantum circuit for given quantum state
 * \param str include desired quantum state for preparation in tt or pla version
 * \param amplitudes a list of non-zero amplitudes in order from MSB to LSB
 * \param stats store all desired statistics of quantum state preparation process
 * \param param specify some parameters for qsp such as creating BDD from tt or pla
*/
template<class Network>
void sparse_qsp( Network& network, std::string str, std::vector<double> amplitudes, sparse_qsp_statistics& stats, create_bdd_param param = {} )
{

  uint32_t num_inputs = log2( str.size() );
  /* reordering tt */
  std::vector<uint32_t> orders;
  for ( int32_t i = num_inputs - 1; i >= 0; i-- )
    orders.emplace_back( i );

  /* create DD */
  Cudd cudd;
  auto f_add = create_dd_from_str(cudd, str, num_inputs, orders, param);

  /* draw ADD in a output file */
  draw_dump( f_add, cudd.getManager() );

  /* Generate quantum gates by traversing ADD */
  //gates_dd_t gates;
  stopwatch<>::duration_type time_add_traversal{ 0 };
  {
    stopwatch t( time_add_traversal );
    //detail::extract_quantum_gates( f_add, num_inputs, gates );
  }
  
  //create_qc_for_MCgates(network, gates[f_add], orders);

  /* extract statistics */
  stats.nodes += Cudd_DagSize( f_add ) - 2; // it consider 2 nodes for "0" and "1"
  stats.time += to_seconds( time_add_traversal );
  //detail::extract_statistics( cudd, f_add, gates, stats, orders );

}

} //namespace angel end