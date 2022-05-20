#pragma once
#define _USE_MATH_DEFINES
#include "common_bdd.hpp"
#include "sparse_qsp.hpp"


namespace angel
{


using gates_sqs_t = std::vector<std::pair<target_qubit, std::vector<uint32_t>>>;


//**************************************************************
//**************************************************************
template<class Network>
void sparse_uniform_qsp( Network& network, DdNode* f_bdd, uint32_t num_inputs, sparse_qsp_statistics& stats, uint64_t& cnot_count)
{
  /* Generate quantum gates by traversing BDD from right to left */
  gates_sqs_t gates;
  stopwatch<>::duration_type time_add_traversal{ 0 };
  {
    stopwatch t( time_add_traversal );
    detail::extract_quantum_gates( f_bdd, num_inputs, gates, stats );
  }

  create_qc_for_sparse_uqsp( network, gates, num_inputs + 1, cnot_count );

  /* extract statistics */
  stats.nodes += Cudd_DagSize( f_bdd ) - 2; // it consider 2 nodes for "0" and "1"
  stats.time += to_seconds( time_add_traversal );
  stats.cnots += cnot_count;
  stats.paths += Cudd_CountPathsToNonZero(f_bdd);
}

/**
 * \breif Sparse Uniform Quantum State Preparation using Binary Decision Diagram
 * 
 * \tparam Network the type of generated quantum circuit
 * \param network the extracted quantum circuit for given quantum state
 * \param str include desired quantum state for preparation in tt or pla version
 * \param stats store all desired statistics of quantum state preparation process
 * \param param specify some parameters for qsp such as creating BDD from tt or pla
*/
template<class Network>
void sparse_uniform_qsp( Network& network, std::string str, sparse_qsp_statistics& stats, uint64_t& cnot_count, create_bdd_param param = {} )
{

  /* reordering tt */
  uint32_t num_inputs = log2( str.size() );
  std::vector<uint32_t> orders;
  for ( int32_t i = num_inputs; i >= 0; i-- )
    orders.emplace_back( i );

  /* create DD */
  Cudd cudd;
  auto f_bdd = create_dd_from_str( cudd, str, num_inputs, orders, param );

  /* draw ADD in a output file */
  draw_dump( f_bdd, cudd.getManager() );

  sparse_uniform_qsp(network, f_bdd, num_inputs, stats, cnot_count);
}


} // namespace angel