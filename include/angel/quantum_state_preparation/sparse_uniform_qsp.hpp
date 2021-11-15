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

struct target_qubit
{
  uint32_t index;
  enum gType {NOT, Ry};
  double angle;
}

using gates_sqs_t = std::vector<std::pair<target_qubit, std::vector<uint32_t>;

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

void count_probabilities_bdd_nodes( std::unordered_set<DdNode*>& visited,
                           std::vector<std::map<DdNode*, double>>& node_prob,
                           DdNode* f, uint32_t num_vars, std::vector<double> amplitudes )
{
  auto current = f;
  if ( visited.count( current ) )
    return;
  if ( Cudd_IsConstant( current ) )
    return;

  count_probabilities_bdd_nodes( visited, node_prob, cuddE( current ), num_vars, amplitudes );
  count_probabilities_bdd_nodes( visited, node_prob, cuddT( current ), num_vars, amplitudes );

  visited.insert( current );
  uint32_t Eones = 0;
  uint32_t Tones = 0;

  if ( Cudd_IsConstant( cuddT( current ) ) )
  {
    if ( Cudd_V( cuddT( current ) ) )
    {
      auto temp_pow = num_vars - current->index - 1;
      Tones = pow( 2, temp_pow ) * 1;
    }
  }
  else
  {
    auto temp_pow = cuddT( current )->index - 1 - current->index;
    auto Tvalue = 0;
    Tvalue = node_ones[cuddT( current )->index].find( cuddT( current ) )->second;
    Tones = pow( 2, temp_pow ) * Tvalue;
  }

  if ( Cudd_IsConstant( cuddE( current ) ) )
  {
    if ( Cudd_V( cuddE( current ) ) )
    {
      auto temp_pow = num_vars - current->index - 1;
      Eones = pow( 2, temp_pow ) * 1;
    }
  }
  else
  {
    auto temp_pow = cuddE( current )->index - 1 - current->index;
    //auto max_ones = pow( 2, num_vars - orders[cuddE( current )->index] );
    auto Evalue = 0;
    Evalue = node_ones[cuddE( current )->index].find( cuddE( current ) )->second;
    Eones = pow( 2, temp_pow ) * Evalue;
  }

  node_ones[current->index].insert( { current, Tones + Eones } );
}

void extract_probabilities_and_MCgates_top_down( std::unordered_set<DdNode*>& visited,
                                                 std::vector<std::map<DdNode*, uint32_t>> node_ones,
                                                 std::vector<uint32_t> controls,
                                                 gates_dd_t & gates,
                                                 DdNode* f, uint32_t num_vars, uint32_t amp_idx )
{
  auto current = f;
  if ( visited.count( current ) )
    return;
  if ( Cudd_IsConstant( current ) )
  {
    return;
  }

  visited.insert( current );

  double ep = 0.0;
  double dp = node_ones[current->index].find( current )->second;
  if ( Cudd_IsConstant( cuddE( current ) ) && Cudd_V( cuddE( current ) ) )
  {
    ep = pow( 2, num_vars - 1 - current->index );
  }
  else if ( Cudd_IsConstant( cuddE( current ) ) && !Cudd_V( cuddE( current ) ) )
  {
    ep = 0.0;
  }
  else
  {
    ep = node_ones[cuddE( current )->index].find( cuddE( current ) )->second *
         ( pow( 2, cuddE( current )->index - current->index - 1 ) );
  }

  /* check there is branch */


  double p = ep / dp; /* zero probability */

  /* inserting current single-qubit G(p) gate */
  if ( p != 1 )
  {
    double angle = 2 * acos( sqrt( p ));
    gates[current][current->index].emplace_back( std::pair{ angle, controls } );
  }

  std::vector<uint32_t> controls_left;
  std::vector<uint32_t> controls_right;
  for ( auto k = 0u; k < controls.size(); k++ )
  {
    controls_left.emplace_back( controls[k] );
    controls_right.emplace_back( controls[k] );
  }
   
  //if ( p != 0 && p != 1 )
  //{
    controls_left.emplace_back( current->index * 2 + 1 );
    controls_right.emplace_back( current->index * 2 );
  //}

  /* left child  */
  if ( Cudd_IsConstant( cuddE( current ) ) && !Cudd_V( cuddE( current ) ) )
    int32_t s = 0; // nothing to do
  else if ( Cudd_IsConstant( cuddE( current ) ) && Cudd_V( cuddE( current ) ) )
  {
    for ( auto i = current->index + 1; i < num_vars; i++ )
    {
      double angle = 2 * acos( sqrt( 1.0 / 2.0 ));
      gates[current][i].emplace_back( std::pair{ angle, controls_left } );
    }
  }
  else
  {

    for ( auto i = current->index + 1; i < cuddE( current )->index; i++ )
    {
      double angle = 2 * acos( sqrt( 1.0 / 2.0 ));
      gates[current][i].emplace_back( std::pair{ angle, controls_left } );
    }
  }

  /* right child  */
  if ( Cudd_IsConstant( cuddT( current ) ) && !Cudd_V( cuddT( current ) ) )
    int32_t s = 0; // nothing to do
  else if ( Cudd_IsConstant( cuddT( current ) ) && Cudd_V( cuddT( current ) ) )
  {
    for ( auto i = current->index + 1; i < num_vars; i++ )
    {
      double angle = 2 * acos( sqrt( 1.0 / 2.0 ));
      gates[current][i].emplace_back( std::pair{ angle, controls_right } );
    }
  }
  else
  {
    for ( auto i = current->index + 1; i < cuddT( current )->index; i++ )
    {
      double angle = 2 * acos( sqrt( 1.0 / 2.0 ));
      gates[current][i].emplace_back( std::pair{ angle, controls_right } );
    }
  }

  extract_probabilities_and_MCgates_top_down( visited, node_ones, controls_left, gates, cuddE( current ), num_vars );

  extract_probabilities_and_MCgates_top_down( visited, node_ones, controls_right, gates, cuddT( current ), num_vars );
}

void extract_quantum_gates( DdNode* f_add, uint32_t num_inputs, gates_sqs_t & gates )
{
  std::vector<std::map<DdNode*, uint32_t>> node_ones( num_inputs );
  std::unordered_set<DdNode*> visited;
  std::unordered_set<DdNode*> visited1;
  count_ones_bdd_nodes( visited, node_ones, f_add, num_inputs );
  extract_probabilities_and_MCgates_bottom_up( visited1, node_ones, gates, f_add, num_inputs );
  //extract_gates_representation( f_add, num_inputs, gates );
}

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

  /* reordering tt */
  uint32_t num_inputs = log2( str.size() );
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
    detail::extract_quantum_gates( f_add, num_inputs, gates );
  }
  
  //create_qc_for_MCgates(network, gates[f_add], orders);

  /* extract statistics */
  stats.nodes += Cudd_DagSize( f_add ) - 2; // it consider 2 nodes for "0" and "1"
  stats.time += to_seconds( time_add_traversal );
  //detail::extract_statistics( cudd, f_add, gates, stats, orders );

}

} //namespace angel end