#pragma once
#include "utils.hpp"
#include <angel/quantum_circuit/create_quantum_circuit.hpp>
#include <angel/utils/helper_functions.hpp>
#include <angel/dependency_analysis/esop_based_dependency_analysis.hpp>
#include <angel/utils/stopwatch.hpp>
#include <cmath>
#include <cplusplus/cuddObj.hh>
#include <cudd/cudd.h>
#include <cudd/cuddInt.h>
#include <fstream>
#include <iostream>
#include <kitty/kitty.hpp>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

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

BDD create_bdd_from_tt_str( Cudd& cudd, std::string tt_str, uint32_t num_inputs )
{
  auto bddNodes = new BDD[num_inputs];
  for ( int i = num_inputs - 1; i >= 0; i-- )
  {
    bddNodes[i] = cudd.bddVar(); // index 0: LSB
  }

  BDD f_bdd;
  int sig = 1;

  /* 
    zero index in tt_str consist the biggest minterm of tt 
  */

  for ( int32_t i = tt_str.size()-1; i >= 0; i-- )
  {
    if ( tt_str[i] == '1' )
    {
      auto n = (tt_str.size()-1) - i;
      BDD temp;
      temp = ( ( n & 1 ) == 1 ) ? bddNodes[0] : !bddNodes[0];
      n >>= 1;
      for ( auto j = 1u; j < num_inputs; j++ )
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

BDD create_bdd_from_minterms( Cudd& cudd, std::map<unsigned long long, float> amplitudes, uint32_t num_inputs )
{
  auto bddNodes = new BDD[num_inputs];
  for ( int i = num_inputs - 1; i >= 0; i-- )
  {
    bddNodes[i] = cudd.bddVar(); // index 0: LSB
  }

  BDD f_bdd;
  int sig = 1;

  /* 
    zero index in tt_str consist the biggest minterm of tt 
  */

  for(auto [idx, amp]: amplitudes)
  {
    auto mint = idx;
    BDD temp;
    temp = ( ( mint & 1 ) == 1 ) ? bddNodes[0] : !bddNodes[0];
    mint >>= 1;
    for ( auto j = 1u; j < num_inputs; j++ )
    {
      temp &= ( ( mint & 1 ) == 1 ) ? bddNodes[j] : !bddNodes[j];
      mint >>= 1;
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

  return f_bdd;
}

BDD create_bdd( Cudd& cudd, std::string str, create_bdd_param bdd_param, uint32_t& num_inputs )
{
  BDD bdd;
  if ( bdd_param.strategy == create_bdd_param::strategy::create_from_tt )
    bdd = create_bdd_from_tt_str( cudd, str, num_inputs );
  else
    bdd = create_bdd_from_pla( cudd, str, num_inputs );

  return bdd;
}

DdNode* create_add( Cudd& cudd, std::map<unsigned long long, float> amplitudes, uint32_t num_inputs)
{
  auto gbm = cudd.getManager();
  DdNode** addNodes = new DdNode*[num_inputs];
  for ( int i = num_inputs - 1; i >= 0; i-- )
  {
    addNodes[i] = Cudd_addNewVar(gbm);  // index 0: LSB
  }

  std::cout<<num_inputs<<std::endl;

  DdNode* f_add, *temp_add;
  bool sig_firstm=true;
  for(auto [idx, amp]: amplitudes)
  {
    std::cout<<std::endl<<"idx:  "<<idx<<"   amp: "<<amp<<std::endl;
    DdNode* minterm, *temp_node;
    minterm = Cudd_addConst (gbm, amp);
    Cudd_Ref(minterm);
    
    for(auto i=0; i<num_inputs; i++)
    {
      auto mint_bit = (idx >> i) & 1;
      if(mint_bit){
        std::cout<<"p"<<i<<"  ";
        temp_node = Cudd_addApply (gbm, Cudd_addTimes, addNodes[i] , minterm); 
        Cudd_Ref(temp_node);
        Cudd_RecursiveDeref(gbm,minterm);
        minterm = temp_node;  
      }   
      else{
        std::cout<<"n"<<i<<"  ";
        temp_node = Cudd_addApply (gbm, Cudd_addTimes, Cudd_addCmpl(gbm, addNodes[i]) , minterm);
        Cudd_Ref(temp_node);
        Cudd_RecursiveDeref(gbm,minterm);
        minterm = temp_node;
      }    
    }

    if(sig_firstm){
      temp_add = minterm;
      Cudd_RecursiveDeref(gbm, minterm);
      Cudd_Ref(temp_add);
      f_add = temp_node;
      sig_firstm = false; 
    }
    else{
       temp_add = Cudd_addApply (gbm, Cudd_addOr, f_add, minterm);
       Cudd_Ref(temp_add);
       Cudd_RecursiveDeref(gbm, minterm);
       Cudd_RecursiveDeref(gbm, f_add);
       f_add = temp_add;
    }

  }

  return f_add;
}

DdNode* create_add_prev( Cudd& cudd, std::map<unsigned long long, float> amplitudes, uint32_t num_inputs)
{
  auto gbm = cudd.getManager();
  DdNode** addNodes = new DdNode*[num_inputs];
  for ( int i = num_inputs - 1; i >= 0; i-- )
  {
    addNodes[i] = Cudd_addNewVar(gbm);  // index 0: LSB
  }

  DdNode* f_add;
  bool sig=1;
  for(auto [idx, amp]: amplitudes)
  {
    DdNode* minterm;
    auto temp = idx & 1;
    if(temp){
      minterm = Cudd_addApply (gbm, Cudd_addTimes, addNodes[0] , Cudd_addConst (gbm, (CUDD_VALUE_TYPE)1)); 
      Cudd_Ref(minterm);
    }    
    else{
      minterm = Cudd_addApply (gbm, Cudd_addTimes, Cudd_addCmpl(gbm, addNodes[0]) , Cudd_addConst (gbm, (CUDD_VALUE_TYPE)1));
      Cudd_Ref(minterm);
    }
      

    uint32_t mint = idx;
    for(auto i=1u; i<num_inputs; i++)
    {
      mint >>=1;
      if(mint & 1){
        Cudd_RecursiveDeref(gbm,minterm);
        minterm = Cudd_addApply (gbm, Cudd_addTimes, addNodes[i] , minterm); 
        Cudd_Ref(minterm);
      }   
      else{
        Cudd_RecursiveDeref(gbm,minterm);
        minterm = Cudd_addApply (gbm, Cudd_addTimes, Cudd_addCmpl(gbm, addNodes[i]) , minterm);
        Cudd_Ref(minterm);
      }
        
    }

    //Cudd_Ref(minterm);

    if ( sig )
    {
      f_add = Cudd_addApply (gbm, Cudd_addTimes, minterm, Cudd_addConst (gbm, (CUDD_VALUE_TYPE)amp));
      Cudd_Ref(f_add);
      Cudd_RecursiveDeref(gbm,minterm);
      sig = 0;
    }
    else
    {
      auto temp = Cudd_addApply (gbm, Cudd_addTimes, minterm, Cudd_addConst (gbm, (CUDD_VALUE_TYPE)amp));
      Cudd_Ref(temp);
      Cudd_RecursiveDeref(gbm,f_add);
      f_add = Cudd_addApply (gbm, Cudd_addPlus, f_add, temp);
      Cudd_Ref(f_add);
      Cudd_RecursiveDeref(gbm, temp);

    }

    //Cudd_RecursiveDeref(gbm, minterm);
  }

  Cudd_RecursiveDeref(gbm,f_add);
  return f_add;
}

void draw_dump( DdNode* f_add, DdManager* mgr )
{
  /* command to run: dot -Tpng graph.dot > output.png */
  FILE* outfile; /* output file pointer for .dot file */
  outfile = fopen( "graph.dot", "w" );
  DdNode** ddnodearray = (DdNode**)malloc( sizeof( DdNode* ) ); /* initialize the function array */
  ddnodearray[0] = f_add;
  Cudd_DumpDot( mgr, 1, ddnodearray, NULL, NULL, outfile ); /* dump the function to .dot file */
}

DdNode * create_dd_from_str( Cudd & cudd, std::string str, uint32_t num_inputs, std::vector<uint32_t> orders, create_bdd_param param)
{
  kitty::dynamic_truth_table tt( num_inputs );
  kitty::create_from_binary_string( tt, str );
  std::vector<uint32_t> mins = kitty::get_minterms( tt );
  reordering_on_tt_inplace( tt, orders );
  
  str = kitty::to_binary( tt );

  /* Create BDD */
  auto mgr = cudd.getManager();
  auto f_bdd = create_bdd( cudd, str, param, num_inputs );
  auto f_add = Cudd_BddToAdd( mgr, f_bdd.getNode() );

  /* 
    BDD help sample 
    auto d = mgr.bddVar(); //MSB
    auto c = mgr.bddVar(); //LSB
  */

  return f_add;
}

void compute_0_probabilities_for_dd_nodes( std::unordered_set<DdNode*>& visited,
                           std::map<DdNode*, double>& node_0_p,
                           DdNode* f, uint32_t num_vars )
{
  auto current = f;
  if ( visited.count( current ) )
    return;
  if ( Cudd_IsConstant( current ) )
    return;

  compute_0_probabilities_for_dd_nodes( visited, node_0_p, cuddE( current ), num_vars );
  compute_0_probabilities_for_dd_nodes( visited, node_0_p, cuddT( current ), num_vars );

  visited.insert( current );
  double Eones = 0.0;
  double Tones = 0.0;

  if ( Cudd_IsConstant( cuddT( current ) ) )
  {
    auto const_value = Cudd_V( cuddT( current ) );
    if ( const_value != 0 )
    {
      auto temp_pow = num_vars - current->index - 1;
      Tones = pow( 2, temp_pow ) * const_value;
    }
  }
  else
  {
    auto temp_pow = cuddT( current )->index - 1 - current->index;
    auto Tvalue = 0;
    Tvalue = node_0_p[cuddT( current )];   
    Tones = pow( 2, temp_pow ) * Tvalue;
  }

  if ( Cudd_IsConstant( cuddE( current ) ) )
  {
    auto const_value = Cudd_V( cuddE( current ) );
    if ( const_value != 0 )
    {
      auto temp_pow = num_vars - current->index - 1;
      Eones = pow( 2, temp_pow ) * const_value;
    }
  }
  else
  {
    auto temp_pow = cuddE( current )->index - 1 - current->index;
    auto Evalue = 0;
    Evalue = node_0_p[cuddE( current )]; 
    Eones = pow( 2, temp_pow ) * Evalue;
  }

  node_0_p[current] = double(Eones/(Tones + Eones)); 
}

kitty::dynamic_truth_table remove_vars_from_tt(kitty::dynamic_truth_table tt, const std::vector<uint32_t> & vars_to_erase)
{
  kitty::dynamic_truth_table tt_current = tt;
  auto num_vars = tt.num_vars();
  for(const auto & var: vars_to_erase)
  {
    auto tt_temp = kitty::cofactor0( tt_current, var ) | kitty::cofactor1( tt_current, var );
    tt_current = tt_temp;
  }
  kitty::min_base_inplace(tt_current);

  auto tt_new = kitty::shrink_to( tt_current, num_vars-vars_to_erase.size() );

  return tt_new;
}


std::string extract_dependencies_compute_tt (std::string tt_str, esop_deps_analysis deps_strategy)
{
  int32_t const num_vars = std::log2( tt_str.size() );
  kitty::dynamic_truth_table tt(num_vars);
  kitty::create_from_binary_string(tt, tt_str);

  /* extract dependencies */
  esop_deps_analysis::result_type deps = deps_strategy.run( tt ); 
  std::cout<<"deps: \n";
  deps.print();

  std::vector<uint32_t> vars_to_erase;

  for(auto [var_idx, others]: deps.dependencies)
  {
    vars_to_erase.emplace_back(var_idx);
  }

  auto smaller_tt = remove_vars_from_tt(tt, vars_to_erase);

  auto ones = kitty::count_ones(smaller_tt);
  auto vars = smaller_tt.num_vars();
  
  return kitty::to_binary(smaller_tt);
}



} //namespace angel end