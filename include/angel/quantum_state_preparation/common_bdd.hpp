

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

BDD create_bdd( Cudd& cudd, std::string str, create_bdd_param bdd_param, uint32_t& num_inputs )
{
  BDD bdd;
  if ( bdd_param.strategy == create_bdd_param::strategy::create_from_tt )
    bdd = create_bdd_from_tt_str( cudd, str, num_inputs );
  else
    bdd = create_bdd_from_pla( cudd, str, num_inputs );

  return bdd;
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

void compute_0_probabilities_for_bdd_nodes( std::unordered_set<DdNode*>& visited,
                           std::map<DdNode*, double>& node_0_p,
                           DdNode* f, uint32_t num_vars )
{
  auto current = f;
  if ( visited.count( current ) )
    return;
  if ( Cudd_IsConstant( current ) )
    return;

  compute_0_probabilities_for_bdd_nodes( visited, node_0_p, cuddE( current ), num_vars );
  compute_0_probabilities_for_bdd_nodes( visited, node_0_p, cuddT( current ), num_vars );

  visited.insert( current );
  double Eones = 0.0;
  double Tones = 0.0;

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
    Tvalue = node_0_p[cuddT( current )];   //node_0_p[cuddT( current )->index].find( cuddT( current ) )->second;
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
    Evalue = node_0_p[cuddE( current )]; //node_0_p[cuddE( current )->index].find( cuddE( current ) )->second;
    Eones = pow( 2, temp_pow ) * Evalue;
  }

  node_0_p[current] = double(Eones/(Tones + Eones)); //[current->index].insert( { current, double(Eones/(Tones + Eones)) } );
}



} //namespace angel end