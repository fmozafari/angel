#pragma once
#define _USE_MATH_DEFINES
#include "common_bdd.hpp"
#include "sparse_uniform_qsp.hpp"


namespace angel
{

/*struct target_qubit
{
  uint32_t index;
  enum gType {NOT, Ry};
  gType gt;
  double angle;
};*/

using gates_sqs_t = std::vector<std::pair<target_qubit, std::vector<uint32_t>>>;

namespace detail
{


} // namespace detail
//**************************************************************
//**************************************************************
template<class Network>
void sparse_qsp(Network& network, DdNode* f_add, uint32_t num_inputs, sparse_qsp_statistics& stats, uint64_t& cnot_count)
{
  /* Generate quantum gates by traversing ADD from biggest minterm to lowest */
  gates_sqs_t gates;
  stopwatch<>::duration_type time_add_traversal{ 0 };
  {
    stopwatch t( time_add_traversal );
    detail::extract_quantum_gates( f_add, num_inputs, gates );
  }

  create_qc_for_sparse_uqsp( network, gates, num_inputs + 1, cnot_count );

  /* extract statistics */
  stats.nodes += Cudd_DagSize( f_add ) - 2; // it consider 2 nodes for "0" and "1"
  stats.time += to_seconds( time_add_traversal );
}

/**
 * \breif Sparse Quantum State Preparation using Algebraic Decision Diagram
 * 
 * \tparam Network the type of generated quantum circuit
 * \param network the extracted quantum circuit for given quantum state
 * \param str include desired quantum state for preparation in tt or pla version
 * \param amplitudes include desired amplitudes of basis states
 * \param stats store all desired statistics of quantum state preparation process
 * \param param specify some parameters for qsp such as creating BDD from tt or pla
*/
template<class Network>
void sparse_qsp( Network& network, std::map<uint32_t, float> amplitudes, uint32_t num_inputs,
                                               sparse_qsp_statistics& stats, uint64_t& cnot_count)
{

  /* reordering tt */
  std::vector<uint32_t> orders;
  for ( int32_t i = num_inputs; i >= 0; i-- )
    orders.emplace_back( i );

  /* create DD */
  Cudd cudd;
  auto f_add = create_add( cudd, amplitudes, num_inputs);

  /* draw ADD in a output file */
  draw_dump( f_add, cudd.getManager() );
  sparse_qsp(network, f_add, num_inputs, stats, cnot_count);
  
}


} // namespace angel