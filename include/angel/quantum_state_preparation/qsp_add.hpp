/* Author: Fereshte */
#pragma once
#include <angel/utils/stopwatch.hpp>
#include <cplusplus/cuddObj.hh>
#include <cudd/cudd.h>
#include <cudd/cuddInt.h>
#include <fstream>
#include <map>
#include <tweedledum/algorithms/synthesis/linear_synth.hpp>
#include <tweedledum/gates/gate_base.hpp>
#include <tweedledum/gates/gate_lib.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/io_id.hpp>
#include <unordered_set>

struct qsp_add_statistics
{
  double time{0};
  uint32_t cnots{0};
  uint32_t sqgs{0};
  uint32_t nodes{0};
  uint32_t MC_gates{0};
  uint32_t ancillaes{0};
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

namespace angel
{

struct create_bdd_param
{
  enum class strategy : uint32_t
  {
    create_from_tt,
    create_from_pla
  } strategy = strategy::create_from_tt;
};

namespace detail
{
BDD create_bdd_from_pla( Cudd& cudd, std::string file_name, uint32_t& num_inputs )
{
  std::ifstream infile( file_name );
  std::string in, out;
  infile >> in >> out;
  num_inputs = std::atoi( out.c_str() );
  BDD output;
  auto bddNodes = new BDD[num_inputs];
  for ( int i = num_inputs - 1; i >= 0; i-- )
  {
    bddNodes[i] = cudd.bddVar(); // index 0: LSB
  }

  bool sig = 1;
  while ( infile >> in >> out )
  {
    BDD tmp, var;
    bool sig2 = 1;

    for ( auto i = 0u; i < num_inputs; ++i )
    {
      if ( in[i] == '-' )
        continue;
      var = bddNodes[i];

      if ( in[i] == '0' )
        var = !var;
      if ( sig2 )
      {
        tmp = var;
        sig2 = 0;
      }
      else
        tmp &= var;
    }

    if ( sig )
    {
      output = tmp;
      sig = 0;
    }
    else
      output |= tmp;
  }
  return output;
}

BDD create_bdd_from_tt( Cudd& cudd, std::string file_name, uint32_t& num_inputs )
{
  std::ifstream infile( file_name );
  std::string tt_str;
  infile >> tt_str;
  std::cout << "tt: " << tt_str << std::endl;
  num_inputs = std::log2( tt_str.size() );
  auto bddNodes = new BDD[num_inputs];
  for ( int i = num_inputs - 1; i >= 0; i-- )
  {
    bddNodes[i] = cudd.bddVar(); // index 0: LSB
  }

  BDD f_bdd;
  int sig = 1;
  for ( auto i = 0u; i < tt_str.size(); i++ )
  {
    if ( tt_str[i] == '1' )
    {
      auto n = i;
      BDD temp;
      temp = ( ( n & 1 ) == 1 ) ? bddNodes[0] : !bddNodes[0];
      n >>= 1;
      for ( auto j = 1; j < num_inputs; j++ )
      {
        temp &= ( ( n & 1 ) == 1 ) ? bddNodes[j] : !bddNodes[j];
        n >>= 1;
      }

      if ( sig )
      {
        f_bdd = temp;
        sig = 0;
      }
      else
      {
        f_bdd |= temp;
      }
    }
  }

  return f_bdd;
}

BDD create_bdd( Cudd& cudd, std::string str, create_bdd_param bdd_param, uint32_t& num_inputs )
{
  BDD bdd;
  if ( bdd_param.strategy == create_bdd_param::strategy::create_from_tt )
    bdd = create_bdd_from_tt( cudd, str, num_inputs );
  else
    bdd = create_bdd_from_pla( cudd, str, num_inputs );

  return bdd;
}

void draw_dump( DdNode* f_add, DdManager* mgr )
{
  FILE* outfile; /* output file pointer for .dot file */
  outfile = fopen( "graph.dot", "w" );
  DdNode** ddnodearray = (DdNode**)malloc( sizeof( DdNode* ) ); /* initialize the function array */
  ddnodearray[0] = f_add;
  Cudd_DumpDot( mgr, 1, ddnodearray, NULL, NULL, outfile ); /* dump the function to .dot file */
}

void count_ones_add_nodes( std::unordered_set<DdNode*>& visited,
                           std::vector<std::map<DdNode*, uint32_t>>& node_ones,
                           DdNode* f, uint32_t num_vars, std::vector<uint32_t> orders )
{
  auto current = f;
  if ( visited.count( current ) )
    return;
  if ( Cudd_IsConstant( current ) )
    return;

  count_ones_add_nodes( visited, node_ones, cuddE( current ), num_vars, orders );
  count_ones_add_nodes( visited, node_ones, cuddT( current ), num_vars, orders );

  visited.insert( current );
  uint32_t Eones = 0;
  uint32_t Tones = 0;

  if ( Cudd_IsConstant( cuddT( current ) ) )
  {
    if ( Cudd_V( cuddT( current ) ) )
    {
      auto temp_pow = num_vars - orders[current->index] - 1;
      Tones = pow( 2, temp_pow ) * 1;
    }
  }
  else
  {
    auto temp_pow = orders[cuddT( current )->index] - 1 - orders[current->index];
    auto Tvalue = 0;
    Tvalue = node_ones[cuddT( current )->index].find( cuddT( current ) )->second;
    Tones = pow( 2, temp_pow ) * Tvalue;
  }

  if ( Cudd_IsConstant( cuddE( current ) ) )
  {
    if ( Cudd_V( cuddE( current ) ) )
    {
      auto temp_pow = num_vars - orders[current->index] - 1;
      Eones = pow( 2, temp_pow ) * 1;
    }
  }
  else
  {
    auto temp_pow = orders[cuddE( current )->index] - 1 - orders[current->index];
    auto max_ones = pow( 2, num_vars - orders[cuddE( current )->index] );
    auto Evalue = 0;
    Evalue = node_ones[cuddE( current )->index].find( cuddE( current ) )->second;
    Eones = pow( 2, temp_pow ) * Evalue;
  }

  node_ones[current->index].insert( {current, Tones + Eones} );
}

void extract_probabilities_and_MCgates( std::unordered_set<DdNode*>& visited,
                                        std::vector<std::map<DdNode*, uint32_t>> node_ones,
                                        std::map<DdNode*, std::vector<std::vector<std::pair<double, std::vector<int32_t>>>>>& gates,
                                        DdNode* f, uint32_t num_vars, std::vector<uint32_t> orders )
{
  /* gates construction
  std::map<DdNode*, std::vector<std::vector<std::pair<double, std::vector<int32_t>>>>>
  map -> for each node
  std::vector<std::vector<std::pair<double, std::vector<int32_t>>>>
  vector1 -> include all qubits
  vector2 -> gates for each qubit
  inner vector -> controls
  */
  auto current = f;
  if ( visited.count( current ) )
    return;
  if ( Cudd_IsConstant( current ) )
    return;

  extract_probabilities_and_MCgates( visited, node_ones, gates, cuddE( current ), num_vars, orders );
  extract_probabilities_and_MCgates( visited, node_ones, gates, cuddT( current ), num_vars, orders );

  visited.insert( current );
  double ep = 0;
  double dp = node_ones[current->index].find( current )->second;
  if ( Cudd_IsConstant( cuddT( current ) ) && Cudd_V( cuddT( current ) ) ) //?? -> one probability
  {
    ep = pow( 2, num_vars - 1 - orders[current->index] );
  }
  else if ( Cudd_IsConstant( cuddT( current ) ) && !Cudd_V( cuddT( current ) ) ) //?? -> one probability
  {
    ep = 0;
  }
  else
  {
    ep = node_ones[cuddT( current )->index].find( cuddT( current ) )->second *
         ( pow( 2, orders[cuddT( current )->index] - orders[current->index] - 1 ) ); //?? -> one probability
  }

  double p = ep / dp;

  auto const it = gates.find( current );
  if ( it == gates.end() )
  {
    gates.emplace( current, std::vector<std::vector<std::pair<double, std::vector<int32_t>>>>( num_vars ) );
  }

  /* inserting current single-qubit G(p) gate */
  if ( p != 0 )
  {
    gates[current][current->index].emplace_back( std::make_pair<double, std::vector<int32_t>>( 1 - p, {} ) );
    std::cout << "index: " << current->index << std::endl;
  }
  /* inserting childs gates */
  if ( !Cudd_IsConstant( cuddE( current ) ) )
  {
    for ( auto i = 0; i < num_vars; i++ )
    {
      if ( gates[cuddE( current )][i].size() != 0 )
      {
        for ( auto j = 0; j < gates[cuddE( current )][i].size(); j++ )
        {
          auto pro = gates[cuddE( current )][i][j].first;
          std::vector<int32_t> controls;
          for ( auto k = 0; k < gates[cuddE( current )][i][j].second.size(); k++ )
            controls.emplace_back( gates[cuddE( current )][i][j].second[k] );
          //std::copy(gates[cuddE(current)][i][j].second.begin() , gates[cuddE(current)][i][j].second.end() , controls);
          //auto controls = gates[cuddE(current)][i][j].second;
          controls.emplace_back( -( current->index + 1 ) );
          gates[current][i].emplace_back( std::make_pair( pro, controls ) );
        }
      }
    }
  }

  if ( !Cudd_IsConstant( cuddT( current ) ) )
  {
    for ( auto i = 0; i < num_vars; i++ )
    {
      if ( gates[cuddT( current )][i].size() != 0 )
      {
        for ( auto j = 0; j < gates[cuddT( current )][i].size(); j++ )
        {
          auto pro = gates[cuddT( current )][i][j].first;
          std::vector<int32_t> controls;
          for ( auto k = 0; k < gates[cuddT( current )][i][j].second.size(); k++ )
            controls.emplace_back( gates[cuddT( current )][i][j].second[k] );
          //std::copy(gates[cuddT(current)][i][j].second.begin() , gates[cuddT(current)][i][j].second.end() , controls);

          //auto controls = gates[cuddT(current)][i][j].second;
          controls.emplace_back( current->index + 1 );
          gates[current][i].emplace_back( std::make_pair( pro, controls ) );
        }
      }
    }
  }

  /* inserting Hadamard gates */
  auto Edown = Cudd_IsConstant( cuddE( current ) ) ? num_vars : orders[cuddE( current )->index];
  auto Tdown = Cudd_IsConstant( cuddT( current ) ) ? num_vars : orders[cuddT( current )->index];
  uint32_t E_num_value = 1;
  uint32_t T_num_value = 1;
  if ( Cudd_IsConstant( cuddE( current ) ) && !Cudd_V( cuddE( current ) ) )
    E_num_value = 0;
  if ( Cudd_IsConstant( cuddT( current ) ) && !Cudd_V( cuddT( current ) ) )
    T_num_value = 0;
  for ( auto i = orders[current->index] + 1; i < Edown; i++ )
  {
    auto id = std::find( orders.begin(), orders.end(), i ) - orders.begin();
    if ( E_num_value != 0 )
    {
      //gates_pre[id].insert({current, std::make_tuple( 1/2.0, nullptr , nullptr ) });
      std::vector<int32_t> temp_c;
      temp_c.emplace_back( -( current->index + 1 ) );
      gates[current][id].emplace_back( std::make_pair( 1 / 2.0, temp_c ) );
    }
  }
  for ( auto i = orders[current->index] + 1; i < Tdown; i++ )
  {
    auto id = std::find( orders.begin(), orders.end(), i ) - orders.begin();
    if ( T_num_value != 0 )
    {
      //gates_pre[id].insert({current, std::make_tuple( 1/2.0, nullptr , nullptr ) });
      std::vector<int32_t> temp_c;
      temp_c.emplace_back( current->index + 1 );
      gates[current][id].emplace_back( std::make_pair( 1 / 2.0, temp_c ) );
    }
  }
}

void extract_quantum_gates( DdNode* f_add, uint32_t num_inputs, std::vector<uint32_t> orders,
                            std::map<DdNode*, std::vector<std::vector<std::pair<double, std::vector<int32_t>>>>>& gates )
{
  std::vector<std::map<DdNode*, uint32_t>> node_ones( num_inputs );
  //std::vector<std::pair<uint32_t, uint32_t>> ones_frac;
  std::unordered_set<DdNode*> visited;
  std::unordered_set<DdNode*> visited1;
  count_ones_add_nodes( visited, node_ones, f_add, num_inputs, orders );
  extract_probabilities_and_MCgates( visited1, node_ones, gates, f_add, num_inputs, orders );
}

void extract_statistics( Cudd cudd, DdNode* f_add,
                         std::map<DdNode*, std::vector<std::vector<std::pair<double, std::vector<int32_t>>>>> gates,
                         qsp_add_statistics& stats )
{
  auto total_MC_gates = 0u;
  auto Rxs = 0;
  auto Rys = 0;
  auto Ts = 0;
  auto CNOTs = 0;
  auto Ancillaes = 0;

  for( auto i = 1u; i < cudd.ReadSize(); i++ )
  {
    total_MC_gates += gates[f_add][i].size();

    if( gates[f_add][i].size() < ( pow( 2, i ) / ( 6 * ( i + 1 ) - 12 ) ) )
    {
      for( auto j = 0u; j < gates[f_add][i].size(); j++ )
      {
        auto cs = gates[f_add][i][j].second.size();

        if( cs == 1 )
        {
          CNOTs += 1;
        }
        else if( cs == 2 )
        {
          CNOTs += 6;
          Ts += 7;
        }
        else if ( cs == 3 )
        {
          CNOTs += 12;
          Ts += 15;
          Ancillaes += 1;
        }
        else
        {
          cs += 1;
          CNOTs += ( 6 * cs - 12 );
          Ts += ( 8 * cs - 17 );
          Ancillaes += ( floor( ( cs - 3 ) / 2 ) );
        }
      }
      Rxs += gates[f_add][i].size() + 1;
    }
    else
    {
      CNOTs += pow( 2, i );
      Rys += pow( 2, i );
    }
  }

  stats.MC_gates += total_MC_gates;
  stats.cnots += CNOTs;
  stats.sqgs += ( Rxs + Rys + Ts );
  stats.ancillaes += Ancillaes;
}

} // namespace detail
//**************************************************************

/**
 * \breif Quantum State Preparation using Decision Diagram
 * 
 * \tparam Network the type of generated quantum circuit
 * \param network the extracted quantum circuit for given quantum state
 * \param filename include desired quantum state for preparation in tt or pla version
 * \param stats store all desired statistics of quantum state preparation process
 * \param param specify some parameters for qsp such as creating BDD from tt or pla
*/
template<class Network>
void qsp_add( Network& network, const std::string file_name, qsp_add_statistics& stats, create_bdd_param param = {} )
{
  uint32_t num_inputs;
  /* Create BDD */
  Cudd cudd;
  auto mgr = cudd.getManager();
  auto f_bdd = detail::create_bdd( cudd, file_name, param, num_inputs );
  auto f_add = Cudd_BddToAdd( mgr, f_bdd.getNode() );

  /* 
    BDD help sample 
    auto d = mgr.bddVar(); //MSB
    auto c = mgr.bddVar(); //LSB
  */

  /* draw add in a output file */
  detail::draw_dump( f_add, mgr );
  /* Generate quantum gates by traversing ADD */
  std::vector<uint32_t> orders;
  for ( auto i = 0u; i < cudd.ReadSize(); i++ )
    orders.emplace_back( Cudd_ReadPerm( mgr, i ) );
  std::map<DdNode*, std::vector<std::vector<std::pair<double, std::vector<int32_t>>>>> gates;
  stopwatch<>::duration_type time_add_traversal{0};
  {
    stopwatch t( time_add_traversal );
    detail::extract_quantum_gates( f_add, num_inputs, orders, gates );
  }

  /* extract statistics */
  stats.nodes += Cudd_DagSize( f_add ) - 2; // it consider 2 nodes for "0" and "1"
  stats.time += to_seconds( time_add_traversal );
  detail::extract_statistics( cudd, f_add, gates, stats );
}

} // namespace angel
