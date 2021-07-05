#pragma once
#define _USE_MATH_DEFINES
#include <cmath> 
#include "utils.hpp"
#include <angel/dependency_analysis/common.hpp>
#include <angel/utils/helper_functions.hpp>
#include <angel/utils/stopwatch.hpp>
#include <angel/quantum_circuit/create_quantum_circuit.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/npn.hpp>
#include <kitty/hash.hpp>
#include <fmt/format.h>

#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>


namespace angel
{
using pattern_based_dependencies_t = std::map<uint32_t, dependency_analysis_types::pattern>;
using esop_based_dependencies_t = std::map<uint32_t, std::vector<std::vector<uint32_t>>>;
using gates_t = std::map<uint32_t, std::vector<std::pair<double, std::vector<uint32_t>>>>;
using order_t = std::vector<uint32_t>;

// std::string const filename1 = fmt::format("qsp_cut_functions_ISCAS_8.txt");
// std::ofstream ofs( filename1 );

struct qsp_general_stats
{
  /* cache declaration */
  std::unordered_map<kitty::dynamic_truth_table, qsp_1bench_stats, kitty::hash<kitty::dynamic_truth_table>> cache;

  stopwatch<>::duration_type total_time{0};
  uint32_t total_bench{0};
  uint32_t has_no_dependencies{0};
  uint32_t no_dependencies_computed{0};
  uint32_t has_dependencies{0};
  uint32_t funcdep_bench_useful{0};
  uint32_t funcdep_bench_notuseful{0};
  uint32_t total_cnots{0};
  uint32_t total_sqgs{0};
  std::vector<std::pair<uint32_t, uint32_t>> gates_count = {};

  void report( std::ostream& os = std::cout ) const
  {
    os << "[i] number of analyzed benchmarks = " << total_bench << std::endl;
    os << fmt::format( "[i] total = no deps exist + no deps found + found deps ::: {} = {} + {} + {}\n",
                       ( has_no_dependencies + no_dependencies_computed +
                         has_dependencies ),
                       has_no_dependencies, no_dependencies_computed,
                       has_dependencies );
    os << fmt::format( "[i] total deps = dep useful + dep not useful ::: {} = {} + {}\n",
                       funcdep_bench_useful + funcdep_bench_notuseful, funcdep_bench_useful, funcdep_bench_notuseful );
    os << fmt::format( "[i] total synthesis time (considering dependencies) = {:8.2f}s\n",
                       to_seconds( total_time ) );

    os << fmt::format( "[i] synthesis result: CNOTs / SQgates = {} / {} \n",
                       total_cnots, total_sqgs );
    /* Compute Avg and SD for CNOTs and SQgates */
    auto sum_f = 0; /// CNOTs: first
    auto sum_s = 0; /// SQgates: second
    for ( auto n = 0u; n < gates_count.size(); n++ )
    {
      sum_f += gates_count[n].first;
      sum_s += gates_count[n].second;
    }

    double avg_f = sum_f / double( gates_count.size() );
    double avg_s = sum_s / double( gates_count.size() );

    auto var_f = 0.0;
    auto var_s = 0.0;
    for ( auto n = 0u; n < gates_count.size(); n++ )
    {
      var_f += ( double( gates_count[n].first ) - avg_f ) * ( double( gates_count[n].first ) - avg_f );
      var_s += ( double( gates_count[n].second ) - avg_s ) * ( double( gates_count[n].second ) - avg_s );
    }
    var_f /= gates_count.size();
    var_s /= gates_count.size();
    auto sd_f = sqrt( var_f );
    auto sd_s = sqrt( var_s );
    os << fmt::format( "[i] Avg Cnots = {}\n", avg_f );
    os << fmt::format( "[i] Var Cnots = {}\n", var_f );
    os << fmt::format( "[i] SD Cnots = {}\n", sd_f );

    os << fmt::format( "[i] Avg SQgates = {}\n", avg_s );
    os << fmt::format( "[i] Var SQgates = {}\n", var_s );
    os << fmt::format( "[i] SD SQgates = {}\n", sd_s );
  }
};


/* with esop based dependencies */
inline void MC_qg_generation( gates_t& gates, uint32_t num_vars, kitty::dynamic_truth_table tt, uint32_t var_index, std::vector<uint32_t> controls,
                       esop_based_dependencies_t dependencies, std::vector<uint32_t> zero_lines, std::vector<uint32_t> one_lines )
{
  /*-----co factors-------*/
  kitty::dynamic_truth_table tt0( var_index );
  kitty::dynamic_truth_table tt1( var_index );

  tt0 = kitty::shrink_to( kitty::cofactor0( tt, var_index ), var_index );
  tt1 = kitty::shrink_to( kitty::cofactor1( tt, var_index ), var_index );

  /*--computing probability gate---*/
  auto c0_ones = kitty::count_ones( tt0 );
  auto c1_ones = kitty::count_ones( tt1 );
  auto tt_ones = kitty::count_ones( tt );
  bool is_const = 0;
  auto it0 = std::find( zero_lines.begin(), zero_lines.end(), var_index );
  auto it1 = std::find( one_lines.begin(), one_lines.end(), var_index );
  if ( it1 != one_lines.end() ) // insert not gate
  {
    if ( gates.find( var_index ) == gates.end() )
    {
      gates[var_index].emplace_back( std::pair{M_PI, std::vector<uint32_t>{}} );
    }
    is_const = 1;
  }
  else if ( it0 != zero_lines.end() ) // inzert zero gate
  {
    is_const = 1;
  }
  else if ( c0_ones != tt_ones )
  { /* == --> identity and ignore */
    double angle = 2 * acos( sqrt( static_cast<double>( c0_ones ) / tt_ones ) );
    //angle *= (180/3.14159265); //in degree
    /*----add probability gate----*/
    auto it = dependencies.find( var_index );
    bool deps_useful = false;
    if ( it != dependencies.end() )
    {
      auto const esop_cnots = esop_gate_cost( dependencies[var_index] ).first;
      auto const upperbound_cost = compute_upperbound_cost( zero_lines, one_lines, num_vars, var_index );
      if ( esop_cnots <= upperbound_cost )
      {
        deps_useful = true;
      }
    }

    if ( gates[var_index].size() == 0 && deps_useful )
    {
      for ( auto const& inner : dependencies[var_index] )
      {
        gates[var_index].emplace_back( std::pair{M_PI, std::vector<uint32_t>{inner}} );
      }
    }

    else if(!deps_useful)
    {
        gates[var_index].emplace_back( std::pair{angle, controls} );
    }
  }

  /*-----qc of cofactors-------*/
  /*---check state---*/
  auto c0_allone = ( c0_ones == pow( 2, tt0.num_vars() ) ) ? true : false;
  auto c0_allzero = ( c0_ones == 0 ) ? true : false;
  auto c1_allone = ( c1_ones == pow( 2, tt1.num_vars() ) ) ? true : false;
  auto c1_allzero = ( c1_ones == 0 ) ? true : false;

  std::vector<uint32_t> controls_new0;
  std::copy( controls.begin(), controls.end(), back_inserter( controls_new0 ) );
  if ( dependencies.find( var_index ) == dependencies.end() && !is_const )
  {
    auto ctrl0 = var_index * 2 + 1; /* negetive control: /2 ---> index %2 ---> sign */
    controls_new0.emplace_back( ctrl0 );
  }
  std::vector<uint32_t> controls_new1;
  std::copy( controls.begin(), controls.end(), back_inserter( controls_new1 ) );
  if ( dependencies.find( var_index ) == dependencies.end() && !is_const )
  {
    auto ctrl1 = var_index * 2 + 0; /* positive control: /2 ---> index %2 ---> sign */
    controls_new1.emplace_back( ctrl1 );
  }

  if ( c0_allone )
  {
    /*---add H gates---*/
    for ( auto i = 0u; i < var_index; i++ )
      gates[i].emplace_back( std::pair{M_PI / 2, controls_new0} );
    /*--check one cofactor----*/
    if ( c1_allone )
    {
      /*---add H gates---*/
      for ( auto i = 0u; i < var_index; i++ )
        gates[i].emplace_back( std::pair{M_PI / 2, controls_new1} );
    }
    else if ( c1_allzero )
    {
      return;
    }
    else
    { /* some 1 some 0 */
      MC_qg_generation( gates, num_vars, tt1, var_index - 1, controls_new1, dependencies, zero_lines, one_lines );
    }
  }
  else if ( c0_allzero )
  {
    /*--check one cofactor----*/
    if ( c1_allone )
    {
      /*---add H gates---*/
      for ( auto i = 0u; i < var_index; i++ )
        gates[i].emplace_back( std::pair{M_PI / 2, controls_new1} );
    }
    else if ( c1_allzero )
    {
      return;
    }
    else
    { /* some 1 some 0 */
      MC_qg_generation( gates, num_vars, tt1, var_index - 1, controls_new1, dependencies, zero_lines, one_lines );
    }
  }
  else
  { /* some 0 some 1 for c0 */
    if ( c1_allone )
    {
      MC_qg_generation( gates, num_vars, tt0, var_index - 1, controls_new0, dependencies, zero_lines, one_lines );
      /*---add H gates---*/
      for ( auto i = 0u; i < var_index; i++ )
        gates[i].emplace_back( std::pair{M_PI / 2, controls_new1} );
    }
    else if ( c1_allzero )
    {
      MC_qg_generation( gates, num_vars, tt0, var_index - 1, controls_new0, dependencies, zero_lines, one_lines );
    }
    else
    { /* some 1 some 0 */
      MC_qg_generation( gates, num_vars, tt0, var_index - 1, controls_new0, dependencies, zero_lines, one_lines );
      MC_qg_generation( gates, num_vars, tt1, var_index - 1, controls_new1, dependencies, zero_lines, one_lines );
    }
  }
}

/* with pattern based dependencies */
inline void MC_qg_generation( gates_t& gates, uint32_t num_vars, kitty::dynamic_truth_table tt, uint32_t var_index, std::vector<uint32_t> controls,
                       pattern_based_dependencies_t dependencies, std::vector<uint32_t> zero_lines, std::vector<uint32_t> one_lines )
{
  /*-----co factors-------*/
  kitty::dynamic_truth_table tt0( var_index );
  kitty::dynamic_truth_table tt1( var_index );

  tt0 = kitty::shrink_to( kitty::cofactor0( tt, var_index ), var_index );
  tt1 = kitty::shrink_to( kitty::cofactor1( tt, var_index ), var_index );

  /*--computing probability gate---*/
  auto c0_ones = kitty::count_ones( tt0 );
  auto c1_ones = kitty::count_ones( tt1 );
  auto tt_ones = kitty::count_ones( tt );
  bool is_const = 0;
  auto it0 = std::find( zero_lines.begin(), zero_lines.end(), var_index );
  auto it1 = std::find( one_lines.begin(), one_lines.end(), var_index );
  if ( it1 != one_lines.end() ) // insert not gate
  {
    if ( gates.find( var_index ) == gates.end() )
    {
      gates[var_index].emplace_back( std::pair{M_PI, std::vector<uint32_t>{}} );
    }
    is_const = 1;
  }
  else if ( it0 != zero_lines.end() ) // inzert zero gate
  {
    is_const = 1;
  }
  else if ( c0_ones != tt_ones )
  { /* == --> identity and ignore */
    double angle = 2 * acos( sqrt( static_cast<double>( c0_ones ) / tt_ones ) );
    //angle *= (180/3.14159265); //in degree
    /*----add probability gate----*/
    auto it = dependencies.find( var_index );

    if ( it != dependencies.end() )
    {
      if ( gates[var_index].size() == 0 )
      {

        if ( dependencies[var_index].first == dependency_analysis_types::pattern_kind::EQUAL )
        {
          if ( dependencies[var_index].second[0] % 2 == 0 ) /* equal operation */
          {
            gates[var_index].emplace_back( std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index].second}} );
          }
          else /* not operation */
          {
            gates[var_index].emplace_back( std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index].second}} );
            gates[var_index].emplace_back( std::pair{M_PI, std::vector<uint32_t>{}} );
          }
        }

        else if ( dependencies[var_index].first == dependency_analysis_types::pattern_kind::XOR )
        {
          for ( auto d_in = 0u; d_in < dependencies[var_index].second.size(); d_in++ )
          {
              gates[var_index].emplace_back( std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index].second[d_in]}} );
          }
        }

        else if ( dependencies[var_index].first == dependency_analysis_types::pattern_kind::XNOR )
        {
          for ( auto d_in = 0u; d_in < dependencies[var_index].second.size(); d_in++ )
          {
              gates[var_index].emplace_back( std::pair{M_PI, std::vector<uint32_t>{dependencies[var_index].second[d_in]}} );
          }
          gates[var_index].emplace_back( std::pair{M_PI, std::vector<uint32_t>{}} );
        }

        else if ( dependencies[var_index].first == dependency_analysis_types::pattern_kind::AND )
        {
          gates[var_index].emplace_back( std::pair{M_PI, dependencies[var_index].second} ); /// insert and
        }

        else if ( dependencies[var_index].first == dependency_analysis_types::pattern_kind::NAND )
        {
          gates[var_index].emplace_back( std::pair{M_PI, dependencies[var_index].second} ); /// insert and
          gates[var_index].emplace_back( std::pair{M_PI, std::vector<uint32_t>{}} ); /// insert not for and
        }
      }
    }

    else
      gates[var_index].emplace_back( std::pair{angle, controls} );
  }

  /*-----qc of cofactors-------*/
  /*---check state---*/
  auto c0_allone = ( c0_ones == pow( 2, tt0.num_vars() ) ) ? true : false;
  auto c0_allzero = ( c0_ones == 0 ) ? true : false;
  auto c1_allone = ( c1_ones == pow( 2, tt1.num_vars() ) ) ? true : false;
  auto c1_allzero = ( c1_ones == 0 ) ? true : false;

  std::vector<uint32_t> controls_new0;
  std::copy( controls.begin(), controls.end(), back_inserter( controls_new0 ) );
  if ( dependencies.find( var_index ) == dependencies.end() && !is_const )
  {
    auto ctrl0 = var_index * 2 + 1; /* negetive control: /2 ---> index %2 ---> sign */
    controls_new0.emplace_back( ctrl0 );
  }
  std::vector<uint32_t> controls_new1;
  std::copy( controls.begin(), controls.end(), back_inserter( controls_new1 ) );
  if ( dependencies.find( var_index ) == dependencies.end() && !is_const )
  {
    auto ctrl1 = var_index * 2 + 0; /* positive control: /2 ---> index %2 ---> sign */
    controls_new1.emplace_back( ctrl1 );
  }

  if ( c0_allone )
  {
    /*---add H gates---*/
    for ( auto i = 0u; i < var_index; i++ )
      gates[i].emplace_back( std::pair{M_PI / 2, controls_new0} );
    /*--check one cofactor----*/
    if ( c1_allone )
    {
      /*---add H gates---*/
      for ( auto i = 0u; i < var_index; i++ )
        gates[i].emplace_back( std::pair{M_PI / 2, controls_new1} );
    }
    else if ( c1_allzero )
    {
      return;
    }
    else
    { /* some 1 some 0 */
      MC_qg_generation( gates, num_vars, tt1, var_index - 1, controls_new1, dependencies, zero_lines, one_lines );
    }
  }
  else if ( c0_allzero )
  {
    /*--check one cofactor----*/
    if ( c1_allone )
    {
      /*---add H gates---*/
      for ( auto i = 0u; i < var_index; i++ )
        gates[i].emplace_back( std::pair{M_PI / 2, controls_new1} );
    }
    else if ( c1_allzero )
    {
      return;
    }
    else
    { /* some 1 some 0 */
      MC_qg_generation( gates, num_vars, tt1, var_index - 1, controls_new1, dependencies, zero_lines, one_lines );
    }
  }
  else
  { /* some 0 some 1 for c0 */
    if ( c1_allone )
    {
      MC_qg_generation( gates, num_vars, tt0, var_index - 1, controls_new0, dependencies, zero_lines, one_lines );
      /*---add H gates---*/
      for ( auto i = 0u; i < var_index; i++ )
        gates[i].emplace_back( std::pair{M_PI / 2, controls_new1} );
    }
    else if ( c1_allzero )
    {
      MC_qg_generation( gates, num_vars, tt0, var_index - 1, controls_new0, dependencies, zero_lines, one_lines );
    }
    else
    { /* some 1 some 0 */
      MC_qg_generation( gates, num_vars, tt0, var_index - 1, controls_new0, dependencies, zero_lines, one_lines );
      MC_qg_generation( gates, num_vars, tt1, var_index - 1, controls_new1, dependencies, zero_lines, one_lines );
    }
  }
}

/* without dependencies */
inline void MC_qg_generation( gates_t& gates, kitty::dynamic_truth_table tt, uint32_t var_index, std::vector<uint32_t> controls,
                       std::vector<uint32_t> zero_lines, std::vector<uint32_t> one_lines )
{
  /*-----co factors-------*/
  kitty::dynamic_truth_table tt0( var_index );
  kitty::dynamic_truth_table tt1( var_index );

  tt0 = kitty::shrink_to( kitty::cofactor0( tt, var_index ), var_index );
  tt1 = kitty::shrink_to( kitty::cofactor1( tt, var_index ), var_index );

  /*--computing probability gate---*/
  auto c0_ones = kitty::count_ones( tt0 );
  auto c1_ones = kitty::count_ones( tt1 );
  auto tt_ones = kitty::count_ones( tt );
  bool is_const = 0;
  auto it0 = std::find( zero_lines.begin(), zero_lines.end(), var_index );
  auto it1 = std::find( one_lines.begin(), one_lines.end(), var_index );
  if ( it1 != one_lines.end() ) // insert not gate
  {
    if ( gates.find( var_index ) == gates.end() )
      gates[var_index].emplace_back( std::pair{M_PI, std::vector<uint32_t>{}} );
    is_const = 1;
  }
  else if ( it0 != zero_lines.end() ) // inzert zero gate
  {
    is_const = 1;
  }
  else if ( c0_ones != tt_ones )
  { /* == --> identity and ignore */
    double angle = 2 * acos( sqrt( static_cast<double>( c0_ones ) / tt_ones ) );
    //angle *= (180/3.14159265); //in degree
    /*----add probability gate----*/

    gates[var_index].emplace_back( std::pair{angle, controls} );
  }

  /*-----qc of cofactors-------*/
  /*---check state---*/
  auto c0_allone = ( c0_ones == pow( 2, tt0.num_vars() ) ) ? true : false;
  auto c0_allzero = ( c0_ones == 0 ) ? true : false;
  auto c1_allone = ( c1_ones == pow( 2, tt1.num_vars() ) ) ? true : false;
  auto c1_allzero = ( c1_ones == 0 ) ? true : false;

  std::vector<uint32_t> controls_new0;
  std::copy( controls.begin(), controls.end(), back_inserter( controls_new0 ) );
  if ( !is_const )
  {
    auto ctrl0 = var_index * 2 + 1; /* negetive control: /2 ---> index %2 ---> sign */
    controls_new0.emplace_back( ctrl0 );
  }
  std::vector<uint32_t> controls_new1;
  std::copy( controls.begin(), controls.end(), back_inserter( controls_new1 ) );
  if ( !is_const )
  {
    auto ctrl1 = var_index * 2 + 0; /* positive control: /2 ---> index %2 ---> sign */
    controls_new1.emplace_back( ctrl1 );
  }

  if ( c0_allone )
  {
    /*---add H gates---*/
    for ( auto i = 0u; i < var_index; i++ )
      gates[i].emplace_back( std::pair{M_PI / 2, controls_new0} );
    /*--check one cofactor----*/
    if ( c1_allone )
    {
      /*---add H gates---*/
      for ( auto i = 0u; i < var_index; i++ )
        gates[i].emplace_back( std::pair{M_PI / 2, controls_new1} );
    }
    else if ( c1_allzero )
    {
      return;
    }
    else
    { /* some 1 some 0 */
      MC_qg_generation( gates, tt1, var_index - 1, controls_new1, zero_lines, one_lines );
    }
  }
  else if ( c0_allzero )
  {
    /*--check one cofactor----*/
    if ( c1_allone )
    {
      /*---add H gates---*/
      for ( auto i = 0u; i < var_index; i++ )
        gates[i].emplace_back( std::pair{M_PI / 2, controls_new1} );
    }
    else if ( c1_allzero )
    {
      return;
    }
    else
    { /* some 1 some 0 */
      MC_qg_generation( gates, tt1, var_index - 1, controls_new1, zero_lines, one_lines );
    }
  }
  else
  { /* some 0 some 1 for c0 */
    if ( c1_allone )
    {
      MC_qg_generation( gates, tt0, var_index - 1, controls_new0, zero_lines, one_lines );
      /*---add H gates---*/
      for ( auto i = 0u; i < var_index; i++ )
        gates[i].emplace_back( std::pair{M_PI / 2, controls_new1} );
    }
    else if ( c1_allzero )
    {
      MC_qg_generation( gates, tt0, var_index - 1, controls_new0, zero_lines, one_lines );
    }
    else
    { /* some 1 some 0 */
      MC_qg_generation( gates, tt0, var_index - 1, controls_new0, zero_lines, one_lines );
      MC_qg_generation( gates, tt1, var_index - 1, controls_new1, zero_lines, one_lines );
    }
  }
}


struct state_preparation_parameters
{
  bool verbose{false};
  bool use_upperbound{true};
}; 

struct state_preparation_statistics
{
  uint64_t num_functions{0};
  uint64_t num_unique_functions{0};
  uint64_t num_cnots{0};
  uint64_t num_sqgs{0};
  stopwatch<>::duration_type time_cache{0};
  stopwatch<>::duration_type time_total{0};

  void reset()
  {
    *this = {};
  }
  void report( std::ostream& os = std::cout ) const
  {
    os << fmt::format( "[i] total number of CNOTs = {}\n", num_cnots);
    os << fmt::format( "[i] total number of single-qubit gates = {}\n", num_sqgs);
    os << fmt::format( "[i] Time = {}\n", to_seconds(time_total));
  }

}; 

struct GatesSeq
{
  gates_t gates;
  std::vector<uint32_t> order;
  std::pair<uint32_t, uint32_t> cnots_sqgs;
};


/**
 * \breif Quantum State Preparation using Functional Dependency
 * 
 * \tparam DependencyAnalysisStrategy specify dependency analysis strategy
 * \tparam ReorderingStrategy specify variable reordering strategy
*/

template<class DependencyAnalysisStrategy, class ReorderingStrategy>
class qsp_depsClass
{
public:
  explicit qsp_depsClass(GatesSeq& ntk, DependencyAnalysisStrategy& dependency_strategy, ReorderingStrategy& order_strategy,
                              state_preparation_parameters const& ps, state_preparation_statistics& st )
    : ntk(ntk)
    , dependency_strategy( dependency_strategy )
    , order_strategy( order_strategy )
    , ps( ps )
    , st( st )
  {
  }

  GatesSeq operator()( kitty::dynamic_truth_table const& tt )
  {
    stopwatch t( st.time_total );
    uint32_t const num_variables = tt.num_vars();

    ++st.num_functions;

    /* check if there is a GatesSeq in the cache for this truth table */
    auto const [key_tt, _1, _2] = call_with_stopwatch( st.time_cache, [&]{
        return num_variables <= 7u ? kitty::exact_p_canonization( tt ) : kitty::sifting_p_canonization( tt );
      });
    auto const it = cache.find( key_tt );
    if ( it != std::end( cache ) )
    {
      //GatesSeq empty;
      //return empty;
      
      st.num_cnots += it->second.cnots_sqgs.first;
      if ( ps.verbose )
      {
        fmt::print( "cached function = {} cnots = {}\n", kitty::to_hex( tt ), it->second.cnots_sqgs.first );
      }
      return it->second;
    }
    
    /* run state preparation for the current truth table */
    std::pair<uint32_t, uint32_t> upperbound = {uint64_t( pow( 2u, num_variables ) - 2u ), uint64_t( pow( 2u, num_variables ) - 1u )};
    std::pair<uint32_t, uint32_t> max = {std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max()};
    std::pair<uint32_t, uint32_t> const ub = ps.use_upperbound ? upperbound : max;
    GatesSeq best_ntk{{},{},ub};
    
    order_strategy.foreach_reordering( tt, [this,&best_ntk]( kitty::dynamic_truth_table const& tt, std::vector<uint32_t> const& perm ){
        GatesSeq ntk = synthesize_network( tt, perm );
        
        if ( ntk.cnots_sqgs.first < best_ntk.cnots_sqgs.first )
        {
          best_ntk = ntk;
        }
        if ( ntk.cnots_sqgs.first == best_ntk.cnots_sqgs.first && best_ntk.gates.size()==0 )
        {
          best_ntk = ntk;
        }
        return ntk.cnots_sqgs.first ;
      });
    /* ensure that re-ordering has been exectued at least once */
    assert( best_ntk.cnots_sqgs.first < std::numeric_limits<uint64_t>::max() );

    /* insert result into cache */
    cache.emplace( key_tt, best_ntk );
    if ( ps.verbose )
    {
      fmt::print( "unique function = {} cnots = {}\n", kitty::to_hex( tt ), best_ntk.cnots_sqgs.first );
    }

    /* update statistics */
    ++st.num_unique_functions;
    st.num_cnots += best_ntk.cnots_sqgs.first;
    st.num_sqgs += best_ntk.cnots_sqgs.second;
    return best_ntk;
  }

  GatesSeq synthesize_network( kitty::dynamic_truth_table const& tt, std::vector<uint32_t> const& perm )
  {
    /* FIXME: treat const0 as a special case */
    if ( kitty::is_const0( tt ) )
    {
      return GatesSeq{{}, perm, std::make_pair(0u, 0u)};
    }

    /* extract dependencies */
    auto const result = dependency_strategy.run( tt );
    //result.print();

    /* construct gates */
    return create_gates( tt, result.dependencies, perm );
  }

  template<typename Dependencies>
  GatesSeq create_gates( kitty::dynamic_truth_table const& tt, Dependencies const& dependencies, std::vector<uint32_t> const& perm )
  {   
    uint32_t const num_variables = tt.num_vars();
    uint32_t const var_index = num_variables - 1;

    std::vector<uint32_t> zero_lines, one_lines;
    extract_independent_vars( zero_lines, one_lines, tt );

    gates_t gates;
    std::vector<uint32_t> cs;
    if ( !dependencies.empty() )
    {
      MC_qg_generation( gates, num_variables, tt, var_index, cs, dependencies, zero_lines, one_lines );      
    }
    else
    {
      MC_qg_generation( gates, tt, var_index, cs, zero_lines, one_lines );
    }

    /* FIXME: compute CNOT costs */
    qsp_1bench_stats st;
    std::map<uint32_t, bool> have_deps;
    for ( auto i = 0u; i < num_variables; i++ )
    {
      if ( dependencies.find( i ) != dependencies.end() )
      {
        have_deps[i] = true;
      }
    }
    gates_statistics( gates, have_deps, num_variables, st );

    return GatesSeq{gates, perm, std::make_pair(st.total_cnots, st.total_sqgs)};
  }

protected:
  GatesSeq& ntk;
  DependencyAnalysisStrategy& dependency_strategy;
  ReorderingStrategy& order_strategy;
  state_preparation_parameters const& ps;
  state_preparation_statistics& st;

  std::unordered_map<kitty::dynamic_truth_table, GatesSeq, kitty::hash<kitty::dynamic_truth_table>> cache;
}; 

/**
 * \breif General quantum state preparation algorithm for any function represantstion, 
 * dependency analysis algorithm, and reordering algorithm.
 * 
 * \tparam Network the type of generated quantum circuit
 * \tparam DependencyAnalysisAlgorithm specifies the method of extracting dependencies. By default, we don't have any dependencies.
 * \tparam ReorderingAlgorithm the way of extracting variable orders
 * \param tt given Boolean function corresponding to the quantum state
 * \param ps store all desired parameters of quantum state preparation process
 * \param st store all desired statistics of quantum state preparation process
*/
template<class Network, class DependencyAnalysisStrategy, class ReorderingStrategy>
void qsp_deps(Network& ntk, DependencyAnalysisStrategy& dependency_strategy, ReorderingStrategy& order_strategy,
              kitty::dynamic_truth_table const& tt, state_preparation_parameters const& ps, state_preparation_statistics& st)
{
  GatesSeq gates_seq;
  qsp_depsClass<DependencyAnalysisStrategy, ReorderingStrategy> qsp (gates_seq, dependency_strategy, order_strategy, ps, st);
  GatesSeq gates_network = qsp(tt);
  //print_gates(gates_network.gates, gates_network.order);
  create_qc_for_MCgates(ntk, gates_network.gates, gates_network.order);
}

} // namespace angel
