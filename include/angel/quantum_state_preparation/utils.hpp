#pragma once

#include <iostream>
#include <math.h>
#include <map>
#include <vector>
#include <angel/utils/stopwatch.hpp>

namespace angel
{
using gates_t = std::map<uint32_t, std::vector<std::pair<double, std::vector<uint32_t>>>>;
struct qsp_1bench_stats
{
  stopwatch<>::duration_type total_time{0};
  uint32_t total_cnots{0};
  uint32_t total_rys{0};
  uint32_t total_nots{0};
  std::pair<uint32_t, uint32_t> gates_count = std::make_pair( 0, 0 );
};

uint32_t compute_upperbound_cost( std::vector<uint32_t> zero_lines, std::vector<uint32_t> one_lines, uint32_t num_vars, uint32_t var_index )
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

uint32_t esop_cnot_cost( std::vector<std::vector<uint32_t>> const& esop )
{
  assert( esop.size() > 0u );
  uint32_t cnots_count = 0;
  /// first AND pattern
  auto const n0 = esop[0].size();
  switch ( n0 )
  {
  case 0:
    cnots_count += 0;
    break;
  case 1:
    cnots_count += 1;
    break;
  default:
    cnots_count += ( 1 << n0 );
    break;
  }

  /// the rest
  for ( auto i = 1u; i < esop.size(); i++ )
  {
    auto const n = esop[i].size();
    switch ( n )
    {
    case 0:
      cnots_count += 0;
      break;
    case 1:
      cnots_count += 1;
      break;
    default:
      cnots_count += ( ( 1 << ( n + 1 ) ) - 2 );
      break;
    }
  }

  return cnots_count;
}

/* with dependencies */
void gates_statistics( gates_t gates, std::map<uint32_t, bool> const& have_dependencies,
                       uint32_t const num_vars, qsp_1bench_stats& stats )
{
  auto total_rys = 0;
  auto total_cnots = 0;
  auto total_nots = 0;
  bool have_max_controls;
  auto n_reduc = 0; /* lines that always are zero or one and so we dont need to prepare them */

  for ( int32_t i = num_vars - 1; i >= 0; i-- )
  {
    auto rys = 0;
    auto cnots = 0;
    auto nots = 0;
    have_max_controls = 0;

    if ( gates.find( i ) == gates.end() )
    {
      n_reduc++;
      continue;
    }
    if ( gates[i].size() == 0 )
    {
      n_reduc++;
      continue;
    }
    if ( gates[i].size() == 1 && gates[i][0].first == M_PI && gates[i][0].second.size() == 0 )
    {
      total_nots++;
      n_reduc++;
      continue;
    }

    /* check deps */
    auto it = have_dependencies.find( i );
    /* there exists deps */
    if ( it != have_dependencies.end() )
    {  
      for ( auto j = 0u; j < gates[i].size(); j++ )
      {
        if ( gates[i][j].second.size() == ( ( num_vars - i - 1 ) - n_reduc ) &&
             gates[i][j].second.size() != 0 ) /* number of controls is max or not? */
        {
          have_max_controls = 1;
          break;
        }

        auto cs = gates[i][j].second.size();
        if ( cs == 0 && ( std::abs( gates[i][j].first - M_PI ) < 0.1 ) )
          nots++;
        else if ( cs == 0 )
          rys++;
        else if ( cs == 1 && ( std::abs( gates[i][j].first - M_PI ) < 0.1 ) )
          cnots++;
        else if(j==0)
        {
          rys += pow( 2, cs );
          cnots += pow( 2, cs );
        }

        else
        {
            rys += pow( 2, cs+1 );
            cnots += pow( 2, cs+1 );
        }
        
      }
      
      if ( have_max_controls || cnots > pow( 2, ( ( num_vars - i - 1 ) - n_reduc ) ) ) /* we have max number of controls */
      {
        if ( i == int( num_vars - 1 - n_reduc ) ) /* first line for preparation */
        {
          cnots = 0;
          rys = 1;
        }
        else if ( gates[i].size() == 1 && ( std::abs( gates[i][0].first - M_PI ) < 0.1 ) && gates[i][0].second.size() == 1 ) // second line for preparation
        {
          cnots = 1;
          rys = 0;
        }
        else /* other lines with more than one control */
        {
          rys = pow( 2, ( ( num_vars - i - 1 ) - n_reduc ) );
          cnots = pow( 2, ( ( num_vars - i - 1 ) - n_reduc ) );
        }
      }
    }
    /* doesn't exist deps */
    else
    {
      for ( auto j = 0u; j < gates[i].size(); j++ )
      {
        if ( gates[i][j].second.size() == ( ( num_vars - i - 1 ) - n_reduc ) &&
             gates[i][j].second.size() != 0 ) /* number of controls is max or not? */
        {
          have_max_controls = 1;
          break;
        }

        auto cs = gates[i][j].second.size();
        if ( cs == 0 && ( std::abs( gates[i][j].first - M_PI ) < 0.1 ) )
          nots++;
        else if ( cs == 0 )
          rys++;
        else if ( cs == 1 && ( std::abs( gates[i][j].first - M_PI ) < 0.1 ) )
          cnots += 1;
        else
        {
          rys += pow( 2, cs );
          cnots += pow( 2, cs );
        }
      }
      if ( have_max_controls || cnots > pow( 2, ( ( num_vars - i - 1 ) - n_reduc ) ) ) /* we have max number of controls */
      {
        if ( i == int( num_vars - 1 - n_reduc ) ) /* first line for preparation */
        {
          cnots = 0;
          rys = 1;
        }
        else if ( gates[i].size() == 1 && ( std::abs( gates[i][0].first - M_PI ) < 0.1 ) && gates[i][0].second.size() == 1 ) // second line for preparation
        {
          cnots = 1;
          rys = 0;
        }
        else /* other lines with more than one control */
        {
          rys = pow( 2, ( ( num_vars - i - 1 ) - n_reduc ) );
          cnots = pow( 2, ( ( num_vars - i - 1 ) - n_reduc ) );
        }
      }
    }

    total_rys += rys;
    total_cnots += cnots;
    total_nots += nots;
  }

  stats.total_cnots += total_cnots;
  stats.total_rys += total_rys;
  stats.total_nots += total_nots;
  stats.gates_count = std::make_pair( total_cnots, total_rys + total_nots );

  if ( have_dependencies.size() > 0 )
  {
    if ( total_cnots < ( pow( 2, num_vars - n_reduc ) - 2 ) )
    {
      //++stats.funcdep_bench_useful;
    }
    else
    {
      //++stats.funcdep_bench_notuseful;
    }
  }

  return;
}

/* without dependencies */
void gates_statistics( gates_t gates, uint32_t const num_vars, qsp_1bench_stats& stats )
{
  auto total_rys = 0;
  auto total_cnots = 0;
  auto total_nots = 0;
  bool have_max_controls;
  auto n_reduc = 0; /* lines that always are zero or one and so we dont need to prepare them */

  for ( int32_t i = num_vars - 1; i >= 0; i-- )
  {
    auto rys = 0;
    auto cnots = 0;
    auto nots = 0;
    have_max_controls = 0;

    if ( gates.find( i ) == gates.end() )
    {
      n_reduc++;
      continue;
    }
    if ( gates[i].size() == 0 )
    {
      n_reduc++;
      continue;
    }
    if ( gates[i].size() == 1 && gates[i][0].first == M_PI && gates[i][0].second.size() == 0 )
    {
      nots++;
      n_reduc++;
      continue;
    }

    for ( auto j = 0u; j < gates[i].size(); j++ )
    {
      if ( gates[i][j].second.size() == ( ( num_vars - i - 1 ) - n_reduc ) &&
           gates[i][j].second.size() != 0 ) /* number of controls is max or not? */
      {
        have_max_controls = 1;
        break;
      }

      auto cs = gates[i][j].second.size();
      if ( cs == 0 && ( std::abs( gates[i][j].first - M_PI ) < 0.1 ) )
        nots++;
      else if ( cs == 0 )
        rys++;
      else if ( cs == 1 && ( std::abs( gates[i][j].first - M_PI ) < 0.1 ) )
        cnots += 1;
      else
      {
        rys += pow( 2, cs );
        cnots += pow( 2, cs );
      }
    }
    if ( have_max_controls || cnots > pow( 2, ( ( num_vars - i - 1 ) - n_reduc ) ) ) /* we have max number of controls */
    {
      if ( i == int( num_vars - 1 - n_reduc ) ) /* first line for preparation */
      {
        cnots = 0;
        rys = 1;
      }
      else if ( gates[i].size() == 1 && ( std::abs( gates[i][0].first - M_PI ) < 0.1 ) && gates[i][0].second.size() == 1 ) // second line for preparation
      {
        cnots = 1;
        rys = 0;
      }
      else /* other lines with more than one control */
      {
        rys = pow( 2, ( ( num_vars - i - 1 ) - n_reduc ) );
        cnots = pow( 2, ( ( num_vars - i - 1 ) - n_reduc ) );
      }
    }

    total_rys += rys;
    total_cnots += cnots;
    total_nots += nots;
  }

  stats.total_cnots += total_cnots;
  stats.total_rys += total_rys;
  stats.total_nots += total_nots;
  stats.gates_count = std::make_pair( total_cnots, total_rys + total_nots );

  return;
}

using gates_t = std::map<uint32_t, std::vector<std::pair<double, std::vector<uint32_t>>>>;
void  print_gates(gates_t gates)
{
    for(auto const& target: gates)
    {
        std::cout<<fmt::format("target idx: {}\n", target.first);
        auto gs = target.second;
        for (auto const& g: gs)
        {
            std::cout<<fmt::format("angle: {} controls: ", (g.first/M_PI) *180);
            for(auto const& c: g.second)
            {
                if(c % 2==0)
                {
                    std::cout<<fmt::format("{} ", c/2);
                }
                else
                {
                    std::cout<<fmt::format("-{} ", c/2);
                }
                
            }
            std::cout<<std::endl;
        }
    }
}

// struct deps_operation_stats
// {
//   /* Be verbose */
//   bool verbose = true;

//   /* Map pattern name to number of occurrencies */
//   mutable std::unordered_map<std::string, uint32_t> pattern_occurrence;

//   void report( std::ostream &os = std::cout ) const
//   {
//     for ( const auto& d : pattern_occurrence )
//     {
//       if ( d.second > 0u || verbose )
//       {
//         os << fmt::format( "[i] number of {:6s} operation: {:7d}\n", d.first, d.second );
//       }
//     }
//   }
// };

// void extract_deps_operation_stats( deps_operation_stats& op_stats, dependencies_t const& deps )
// {
//   for( auto i = 0u; i < deps.size(); ++i )
//   {
//     if ( deps.find( i ) == deps.end() )
//       continue;

//     op_stats.pattern_occurrence[fmt::format( "{}-{}", deps.at( i ).first, deps.at( i ).second.size() )]++;
//   }
// }

} /* namespace angel end */