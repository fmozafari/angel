#pragma once

#define _USE_MATH_DEFINES
#include <math.h> 
#include <angel/utils/stopwatch.hpp>

#include <fmt/format.h>

#include <iostream>
#include <map>
#include <vector>

namespace angel
{
using gates_t = std::map<uint32_t, std::vector<std::pair<double, std::vector<uint32_t>>>>;
struct qsp_1bench_stats
{
  stopwatch<>::duration_type total_time{0};
  uint32_t total_cnots{0};
  uint32_t total_sqgs{0};
  std::pair<uint32_t, uint32_t> gates_count = std::make_pair( 0, 0 );
};

inline uint32_t compute_upperbound_cost( std::vector<uint32_t> zero_lines, std::vector<uint32_t> one_lines, uint32_t num_vars, uint32_t var_index )
{
  auto const_lines = 0;
  for ( auto const& zero : zero_lines )
  {
    if ( zero > var_index )
      const_lines++;
  }

  for ( auto const& one : one_lines )
  {
    if ( one > var_index )
      const_lines++;
  }

  auto cost = pow( 2, ( num_vars - var_index - 1 - const_lines ) );

  return cost;
}

inline std::pair<uint32_t, uint32_t> esop_gate_cost( std::vector<std::vector<uint32_t>> const& esop )
{
  assert( esop.size() > 0u );
  uint32_t cnots_count = 0;
  uint32_t sqgs_count = 0;
  /// first AND pattern
  auto const n0 = esop[0].size();
  switch ( n0 )
  {
  case 0:
    cnots_count += 0;
    sqgs_count +=1;
    break;
  case 1:
    cnots_count += 1;
    break;
  default:
    cnots_count += ( 1 << n0 );
    sqgs_count += ( 1 << n0 );
    break;
  }

  if(esop.size() == 1)
    return std::make_pair(cnots_count, sqgs_count);

  /// the rest
  for ( auto i = 1u; i < esop.size(); i++ )
  {
    auto const n = esop[i].size();
    switch ( n )
    {
    case 0:
      cnots_count += 0;
      sqgs_count +=1;
      break;
    case 1:
      cnots_count += 1;
      break;
    default:
      cnots_count += ( ( 1 << ( n + 1 ) ) - 2 );
      sqgs_count += ( ( 1 << ( n + 1 ) ) - 2 );
      break;
    }
  }

  /* using uniformly-controlled gates */
  uint32_t cnots_count2 = 0u;
  uint32_t sqgs_count2 = 0;
  std::vector<uint32_t> controls_idx;
  for(auto k=0u; k<esop[0].size(); k++)
    controls_idx.emplace_back(esop[0][k]/2);

  for(auto i=1u; i<esop.size(); i++)
  {
    for(auto j=0u; j<esop[i].size(); j++)
    {
      auto f = std::find(controls_idx.begin(), controls_idx.end(), esop[i][j]/2);
      if(f == controls_idx.end())
        return std::make_pair(cnots_count, sqgs_count);
    }
  }
  cnots_count2 = pow(2, esop[0].size());
  
  return (cnots_count > cnots_count2) ? std::make_pair(cnots_count2, sqgs_count2) : std::make_pair(cnots_count, sqgs_count);
}

inline std::pair<uint32_t, uint32_t> uniform_gate_cost( std::vector<std::vector<uint32_t>> const& us )
{
  std::vector<uint32_t> controls_idx;
  for(auto const& u : us)
  {
    for(auto i=0u; i<u.size(); i++)
    {
      uint32_t control = u[i]/2;
      auto it = std::find(controls_idx.begin(), controls_idx.end(), control);
      if(it == controls_idx.end())
        controls_idx.emplace_back(control);
    }
  }

  uint32_t cnots = pow(2, controls_idx.size());
  uint32_t sqgs = pow(2, controls_idx.size());

  return std::make_pair(cnots, sqgs);
}

/* with dependencies */
inline void gates_statistics( gates_t gates, std::map<uint32_t, bool> const& have_dependencies,
                       uint32_t const num_vars, qsp_1bench_stats& stats )
{
  auto total_sqgs = 0u;
  auto total_cnots = 0u;
  //auto n_reduc = 0; /* lines that always are zero or one and so we dont need to prepare them */

  for ( int32_t i = num_vars - 1; i >= 0; i-- )
  {
    auto sqgs = 0u;
    auto cnots = 0u;

    if ( gates.find( i ) == gates.end() )
    {
      //n_reduc++;
      continue;
    }
    if ( gates[i].size() == 0 )
    {
      //n_reduc++;
      continue;
    }
    if ( gates[i].size() == 1 && gates[i][0].first == M_PI && gates[i][0].second.size() == 0 )
    {
      total_sqgs++;
      //n_reduc++;
      continue;
    }

    /* check deps */
    auto it = have_dependencies.find( i );
    /* there exists deps */
    if ( it != have_dependencies.end() )
    {
      std::vector<std::vector<uint32_t>> deps;
      for ( auto j = 0u; j < gates[i].size(); j++ )
      {
        deps.emplace_back(gates[i][j].second);
      }    
      auto gates_cost = esop_gate_cost(deps);
      cnots = gates_cost.first;
      sqgs = gates_cost.second;
    }

    /* doesn't exist deps */
    else
    {
      if(gates[i].size() == 1)
      {
        if(gates[i][0].second.size() == 0)
          sqgs = 1;
        else if(gates[i][0].second.size() == 1 && ( std::abs( gates[i][0].first - M_PI ) < 0.1 ))
          cnots = 1;
        else 
        {
          cnots = pow(2, gates[i][0].second.size());
          sqgs = pow(2, gates[i][0].second.size());
        }
      }
      else
      {
        std::vector<std::vector<uint32_t>> us;
        for ( auto j = 0u; j < gates[i].size(); j++ )
        {
            // std::cout<<"i: "<<i<<"  cs: "<<gates[i][j].second.size()<<" target: "<<gates[i][j].first<<std::endl;
            // for(auto m=0; m<gates[i][j].second.size(); m++)
            //     std::cout<<gates[i][j].second[m]/2<<"  ";
            // std::cout<<std::endl<<std::endl;
          us.emplace_back(gates[i][j].second);
        }    
        auto gates_cost = uniform_gate_cost(us);
        cnots = gates_cost.first;
        sqgs = gates_cost.second;
      }
         
    }

    total_sqgs += sqgs;
    total_cnots += cnots;
  }

  stats.total_cnots += total_cnots;
  stats.total_sqgs += total_sqgs;
  stats.gates_count = std::make_pair( total_cnots, total_sqgs );

  return;
}

using gates_t = std::map<uint32_t, std::vector<std::pair<double, std::vector<uint32_t>>>>;
inline void print_gates( gates_t gates, std::vector<uint32_t> order )
{
  for ( auto const& target : gates )
  {
    std::cout << fmt::format( "target idx: {}\n", order[target.first] );
    auto gs = target.second;
    for ( auto const& g : gs )
    {
      std::cout << fmt::format( "angle: {} controls: ", ( g.first / M_PI ) * 180 );
      for ( auto const& c : g.second )
      {
        if ( c % 2 == 0 )
        {
          std::cout << fmt::format( "{} ", order[c / 2] );
        }
        else
        {
          std::cout << fmt::format( "-{} ", order[c / 2] );
        }
      }
      std::cout << std::endl;
    }
  }
}

inline uint32_t extract_max_controls (std::vector< std::vector<uint32_t> > mcs)
{
  std::vector<uint32_t> cs;
  for(auto const& mc : mcs)
  {
    for(auto i=0u; i<mc.size(); i++)
    {
      auto it = std::find(cs.begin(), cs.end(), mc[i]/2 );
      if(it == cs.end())
      {
        cs.emplace_back(mc[i]/2);
      }
    }
  }

  return cs.size();
}

} // namespace angel
