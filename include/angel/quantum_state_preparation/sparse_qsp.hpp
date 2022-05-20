#pragma once
#define _USE_MATH_DEFINES
#include "common_bdd.hpp"


namespace angel
{


using gates_sqs_t = std::vector<std::pair<target_qubit, std::vector<uint32_t>>>;

struct sparse_qsp_statistics
{
  double time{ 0 };
  uint32_t cnots{ 0 };
  uint32_t sqgs{ 0 };
  uint32_t nodes{ 0 };
  uint32_t MC_gates{ 0 };
  uint32_t ancillaes{ 0 };
  uint32_t paths{0};
  uint32_t reducedNodes {0};
  void report( std::ostream& os = std::cout ) const
  {
    os << "[i] time: " << time << std::endl;
    os << "[i] nodes: " << nodes << std::endl;
    os << "[i] paths: " << paths << std::endl;
    os << "[i] reduced nodes: " << reducedNodes << std::endl;
    os << "[i] ancillaes: " << ancillaes << std::endl;
    os << "[i] MC_gates: " << MC_gates << std::endl;
    os << "[i] cnots: " << cnots << std::endl;
    os << "[i] sqgs: " << sqgs << std::endl;
  }
};

namespace detail
{

void extract_MCgates_1child_to_0chid( //std::unordered_set<DdNode*>& visited,
                                      std::map<DdNode*, double> node_0p,
                                      int32_t lastOne_idx,
                                      gates_sqs_t& gates,
                                      DdNode* f, uint32_t num_qubits, std::vector<uint32_t> ancilla_controls, bool& first_amp, sparse_qsp_statistics& stats )
{
  auto current = f;

  //if ( visited.count( current ) )
  //return;

  if ( Cudd_IsConstant( current ) )
  {
    if ( Cudd_V( current ) != 0 ) /* computing ancilla qubits */
    {
      first_amp = false;
      target_qubit tq;
      tq.index = num_qubits;
      tq.gt = target_qubit::NOT;
      tq.angle = 2 * acos( sqrt( 0 ) );

      auto [tqubit, controls] = gates.back();

      if(tqubit.index == num_qubits) /* there are some don't cares -> update controls */
      {
        std::vector<uint32_t> update_controls;
        for(auto i=0u; i<controls.size(); i++)
        {
          if(controls[i]==ancilla_controls[i])
            update_controls.emplace_back(controls[i]);
        }

        gates.pop_back();
        gates.emplace_back( std::make_pair( tq, update_controls ) );
      }
        
      else
        gates.emplace_back( std::make_pair( tq, ancilla_controls ) );
    }
    return;
  }

  //visited.insert( current );

  /* check there is branch */
  auto const0_0child = Cudd_IsConstant( cuddE( current ) ) && (Cudd_V( cuddE( current ) )==0);
  auto const0_1child = Cudd_IsConstant( cuddT( current ) ) && (Cudd_V( cuddT( current ) )==0);

  std::vector<uint32_t> A_controls_0child;
  std::vector<uint32_t> A_controls_1child;
  A_controls_0child.assign( ancilla_controls.begin(), ancilla_controls.end() );
  A_controls_1child.assign( ancilla_controls.begin(), ancilla_controls.end() );

  target_qubit tq;
  std::vector<uint32_t> controls;
  if ( !first_amp )
    controls.emplace_back( (num_qubits)*2 );
  if ( lastOne_idx >= 0 )
    controls.emplace_back( lastOne_idx * 2 );

  if ( !( const0_0child || const0_1child ) ) /* branch */
  {
    tq.index = current->index + 1;
    tq.gt = target_qubit::Ry;
    tq.angle = 2 * acos( sqrt( node_0p[current] ) );

    //if ( lastOne_idx >= 0 )
    //  controls.emplace_back( lastOne_idx * 2 );
    gates.emplace_back( std::make_pair( tq, controls ) );
    A_controls_0child.emplace_back( (current->index+1) * 2 + 1 );
    A_controls_1child.emplace_back( (current->index+1) * 2 );
  }
  else if ( const0_0child ) /* not branch -> apply MC NOT gate */
  {
    tq.index = current->index + 1;
    tq.gt = target_qubit::NOT;
    tq.angle = 2 * acos( sqrt( 0 ) );
    //if ( lastOne_idx >= 0 )
    //  controls.emplace_back( lastOne_idx * 2 );
    gates.emplace_back( std::make_pair( tq, controls ) );
  }

  /* inserting Hadamard gates */
  auto Tdown = Cudd_IsConstant( cuddT( current ) ) ? num_qubits-1 : cuddT( current )->index;
  uint32_t T_num_value = 1;
  if ( Cudd_IsConstant( cuddT( current ) ) && (Cudd_V( cuddT( current ) )==0) )
    T_num_value = 0;
  for ( auto i = current->index + 1; i < Tdown; i++ )
  {
    if ( T_num_value != 0 )
    {
      stats.reducedNodes ++;
      target_qubit tq;
      tq.index = i+1;
      tq.gt = target_qubit::Ry;
      tq.angle = 2 * acos( sqrt( 1.0 / 2.0 ) );
      std::vector<uint32_t> controlsT;
      if ( !first_amp )
        controlsT.emplace_back( (num_qubits)*2 );
      controlsT.emplace_back( (current->index+1) * 2 );
      gates.emplace_back( std::make_pair( tq, controlsT ) );
    }
  }
  extract_MCgates_1child_to_0chid( /*visited,*/ node_0p, current->index+1, gates, cuddT( current ), num_qubits, A_controls_1child, first_amp, stats );

  /* inserting Hadamard gates */
  auto Edown = Cudd_IsConstant( cuddE( current ) ) ? num_qubits-1 : cuddE( current )->index;
  uint32_t E_num_value = 1;
  if ( Cudd_IsConstant( cuddE( current ) ) && (Cudd_V( cuddE( current ) )==0) )
    E_num_value = 0;
  for ( auto i = current->index + 1; i < Edown; i++ )
  {
    if ( E_num_value != 0 )
    {
      stats.reducedNodes ++;
      target_qubit tq;
      tq.index = i+1;
      tq.gt = target_qubit::Ry;
      tq.angle = 2 * acos( sqrt( 1.0 / 2.0 ) );
      gates.emplace_back( std::make_pair( tq, controls ) );
    }
  }
  extract_MCgates_1child_to_0chid( /*visited,*/ node_0p, lastOne_idx, gates, cuddE( current ), num_qubits, A_controls_0child, first_amp , stats);

} // end function

void extract_quantum_gates( DdNode* f_bdd, uint32_t num_inputs, gates_sqs_t& gates, sparse_qsp_statistics& stats )
{
  std::map<DdNode*, double> node_0_p;
  std::unordered_set<DdNode*> visited;
  std::unordered_set<DdNode*> visited1;
  std::vector<uint32_t> a_controlls;
  compute_0_probabilities_for_dd_nodes( visited, node_0_p, f_bdd, num_inputs );
  bool first_amp = true;
  extract_MCgates_1child_to_0chid( /*visited1,*/ node_0_p, -1, gates, f_bdd, num_inputs+1, a_controlls, first_amp, stats );
}

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
    detail::extract_quantum_gates( f_add, num_inputs, gates, stats );
  }

  create_qc_for_sparse_uqsp( network, gates, num_inputs + 1, cnot_count );

  /* extract statistics */
  stats.nodes += Cudd_DagSize( f_add ) - 2; // it consider 2 nodes for "0" and "1"
  stats.time += to_seconds( time_add_traversal );
  stats.cnots += cnot_count;
  stats.paths += Cudd_CountPathsToNonZero(f_add);
  stats.MC_gates += (stats.paths-1);
  stats.ancillaes += 1;
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
void sparse_qsp( Network& network, std::map<unsigned long long, float> amplitudes, uint32_t num_inputs,
                                               sparse_qsp_statistics& stats, uint64_t& cnot_count)
{

  /* reordering tt */
  std::vector<uint32_t> orders;
  for ( int32_t i = num_inputs; i >= 0; i-- )
    orders.emplace_back( i );

  /* create DD */
  Cudd cudd;
  auto f_add = create_add( cudd, amplitudes, num_inputs);

  std::cout<<"after creating ADD\n";
  /* draw ADD in a output file */
  draw_dump( f_add, cudd.getManager() );
  sparse_qsp(network, f_add, num_inputs, stats, cnot_count);
  
}


} // namespace angel