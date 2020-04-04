/* angel: C++ state preparation library
 * Copyright (C) 2019-2020  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*!
  \file dependency_analysis2.hpp

  \brief Dependency analysis algorithm

  \author Fereshte Mozafari
  \author Heinz Riener
*/

#pragma once

#include "common.hpp"

#include <kitty/implicant.hpp>
#include <kitty/partial_truth_table.hpp>
#include <kitty/properties.hpp>

#include <fmt/format.h>

#include <map>
#include <vector>

namespace angel
{

struct dependency_analysis2_params
{
};

struct dependency_analysis2_stats
{
  stopwatch<>::duration_type total_time{0};

  uint32_t num_patterns{0};

  void report() const
  {
  }

  void reset()
  {
    *this = {};
  }
};

struct dependency_analysis2_result_type
{
};

class dependency_analysis2_impl
{
public:
  using parameter_type = dependency_analysis2_params;
  using statistics_type = dependency_analysis2_stats;
  using result_type = dependency_analysis2_result_type;

public:
  using function_type = kitty::dynamic_truth_table;

public:
  explicit dependency_analysis2_impl( dependency_analysis2_params const& ps, dependency_analysis2_stats& st )
    : ps( ps )
    , st( st )
  {
  }

  dependency_analysis2_result_type run( function_type const& function )
  {
    stopwatch t( st.total_time );

    /* create column vectors */
    uint32_t const num_vars = function.num_vars();
    std::vector<dependency_analysis_types::column> columns{num_vars};
    for ( auto i = 0u; i < columns.size(); ++i )
    {
      columns[i].index = i;
    }

    kitty::dynamic_truth_table minterm{function.num_vars()};
    for ( auto const &m : kitty::get_minterms( function ) )
    {
      minterm._bits[0] = m;
      for ( auto i = 0; i < minterm.num_vars(); ++i )
      {
        columns[i].tt.add_bit( kitty::get_bit( minterm, i ) );
      }
    }

    // for ( const auto& c : columns )
    // {
    //   kitty::print_binary( c.tt ); std::cout << std::endl;
    // }

    /* collect divisors */
    std::vector<dependency_analysis_types::column> columns_copy;
    for ( auto i = 0u; i < columns.size(); ++i )
    {
      /* copy the original columns */
      columns_copy = std::vector<dependency_analysis_types::column>( std::begin( columns ) + i, std::end( columns ) );

      /* initialize entropy field */
      {
        auto& target = columns_copy[0];
        target.entropy = std::numeric_limits<uint64_t>::max();

        for ( auto j = 1u; j < columns_copy.size(); ++j )
        {
          auto& sig = columns_copy[j];
          sig.entropy = kitty::relative_distinguishing_power( sig.tt, target.tt );
        }
      }

      /* sort by entropy (highest entropy first) */
      std::sort( std::rbegin( columns_copy ), std::rend( columns_copy ), [&]( auto const &a, auto const &b ) {
          return a.entropy < b.entropy || (a.entropy == b.entropy && a.index > b.index);
        });

      /* overwrite the entropy of the target */
      auto& target = columns_copy[0];
      target.entropy = kitty::absolute_distinguishing_power( target.tt );

      /* print */
      // std::cout << "===========================================================================" << std::endl;
      // std::cout << "target = "; kitty::print_binary( target.tt ); std::cout << ' ' << target.entropy << std::endl;
      //
      // for ( auto j = 1u; j < columns_copy.size(); ++j )
      // {
      //   std::cout << columns_copy[j].index << ' '; kitty::print_binary( columns_copy[j].tt ); std::cout << ' ' << columns_copy[j].entropy << std::endl;
      // }

      /* try to cover the target using the columns */
      uint32_t current_entropy;
      std::vector<uint32_t> indices;

      bool found = false;
      for ( auto j = 1u; j < columns_copy.size() && !found; ++j )
      {
        current_entropy = 0u;
        indices.clear();

        for ( auto k = j; k < columns_copy.size() && !found; ++k )
        {
          indices.push_back( columns_copy[k].index );
          current_entropy += columns_copy[k].entropy;

          if ( current_entropy >= target.entropy )
          {
            auto const success = on_candidate( columns, target.index, indices );
            if ( success )
            {
              ++st.num_patterns;
              found = true;
              break;
            }
          }
        }
      }
    }

    dependency_analysis2_result_type result;
    return result;
  }

private:
  bool on_candidate( std::vector<dependency_analysis_types::column> const& columns, uint32_t target_index, std::vector<uint32_t> const& divisor_indices )
  {
    std::vector<kitty::partial_truth_table> functions;
    for ( const auto& i : divisor_indices )
    {
      functions.push_back( columns[i].tt );
    }

    auto const cover = cover_with_divisors<true>( columns[target_index].tt, functions );
    if ( cover.available )
    {
      fmt::print( "algorithm 1: found a {}-input solution for target {}: ", divisor_indices.size(), target_index );
      for ( const auto& i : divisor_indices )
      {
        std::cout << i << ' ';
      }
      std::cout << std::endl;

      std::cout << "ON-set SOP cover = ";
      for ( const auto& c : cover.cover )
      {
        c.print( functions.size() );
        std::cout << ' ';
      }
      std::cout << std::endl;

      auto const cover2 = cover_with_divisors<true>( ~columns[target_index].tt, functions );
      if ( cover2.available )
      {
        std::cout << "OFF-set SOP cover = ";
        for ( const auto& c : cover2.cover )
        {
          c.print( functions.size() );
          std::cout << ' ';
        }
        std::cout << std::endl;
      }

      return true;
    }

    return false;
  }

  template<bool compute_cover>
  struct cover_result_type;

  template<>
  struct cover_result_type<false>
  {
    bool available;
    uint32_t index;
  };

  template<>
  struct cover_result_type<true>
  {
    bool available;
    uint32_t index;
    std::vector<kitty::cube> cover;
  };

  template<bool compute_cover=false>
  cover_result_type<compute_cover> cover_with_divisors( kitty::partial_truth_table const& target, std::vector<kitty::partial_truth_table> const& divisors )
  {
    std::vector<kitty::cube> cover;

    /* iterate over all bit pairs of target */
    for ( uint32_t i = 0u; i < uint32_t( target.num_bits() ); ++i )
    {
      for ( uint32_t j = i + 1u; j < uint32_t( target.num_bits() ); ++j )
      {
        if ( get_bit( target, i ) != get_bit( target, j ) )
        {
          /* check if this bit pair is distinguished by a divisor */
          bool found = false;
          for ( const auto& d : divisors )
          {
            if ( get_bit( d, i ) != get_bit( d, j ) )
            {
              found = true;
              break;
            }
          }

          if ( !found )
          {
            if constexpr ( compute_cover )
            {
              return {false, i, cover};
            }
            else
            {
              return {false, i};
            }
          }
        }
      }

      if constexpr ( compute_cover )
      {
        if ( !get_bit( target, i ) )
          continue;

        /* compute a cube for the i-th bit */
        kitty::cube c;
        for ( auto j = 0u; j < divisors.size(); ++j )
        {
          c.add_literal( j, get_bit( divisors.at( j ), i ) );
        }
        cover.push_back( c );
      }
    }

    if constexpr ( compute_cover )
    {
      /* remove duplicate cubes */
      std::sort( std::begin( cover ), std::end( cover ) );
      cover.erase( std::unique( std::begin( cover ), std::end( cover ) ), std::end( cover ) );
      return { true, 0u, cover };
    }
    else
    {
      return { true, 0u };
    }
  }

private:
  dependency_analysis2_params const& ps;
  dependency_analysis2_stats& st;
};

} /* namespace angel */
