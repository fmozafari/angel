/* angel: C++ state preparation library
 * Copyright (C) 2018-2019  EPFL
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
  \file dependency_analysis.hpp

  \brief Dependency analysis algorithm

  \author Fereshte Mozafari
  \author Heinz Riener
*/

#pragma once

#include <kitty/implicant.hpp>
#include <kitty/partial_truth_table.hpp>

#include <fmt/format.h>

#include <map>
#include <vector>

namespace angel
{

struct dependency_analysis_params
{
}; /* dependency_analysis_params */

struct dependency_analysis_stats
{
  void report() const
  {
  }

  void reset()
  {
  }
}; /* dependency_analysis_stats */

struct dependency_analysis_types
{
  /* pattern */
  enum class pattern_kind
  {
    /* unary */
    EQUAL = 1,
    /* nary */
    XOR   = 2,
    AND   = 3,
  };

  using fanins = std::vector<uint32_t>;
  using pattern = std::pair<pattern_kind, fanins>;

  /* column */
  struct column
  {
    kitty::partial_truth_table tt;
    uint32_t index;
  };
}; /* dependency_analysis_types */

struct dependency_analysis_result_type
{
  /* maps an index to a dependency pattern, fanins are encoded as literals */
  std::map<uint32_t, dependency_analysis_types::pattern> dependencies;
}; /* dependency_analysis_result_type */

class dependency_analysis_impl
{
public:
  using function_type = kitty::dynamic_truth_table;

public:
  explicit dependency_analysis_impl( dependency_analysis_params const& ps, dependency_analysis_stats &st )
    : ps( ps )
    , st( st )
  {
  }

  dependency_analysis_result_type run( function_type const& function )
  {
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

    for ( auto i = 0u; i < num_vars; ++i )
    {
      for ( auto j = i + 1u; j < num_vars; ++j )
      {
        check_unary_patterns( columns, i, j );

        for ( auto k = j + 1u; k < num_vars; ++k )
        {
          check_nary_patterns( columns, i, { j, k } );

          for ( auto l = k + 1u; l < num_vars; ++l )
          {
            check_nary_patterns( columns, i, { j, k, l } );

            for ( auto m = l + 1u; m < num_vars; ++m )
            {
              check_nary_patterns( columns, i, { j, k, l, m } );

              for ( auto n = m + 1u; n < num_vars; ++n )
              {
                check_nary_patterns( columns, i, { j, k, l, m, n } );
              }
            }
          }
        }
      }
    }

    dependency_analysis_result_type result;
    /* TODO */
    return result;
  }

private:
  void check_unary_patterns( std::vector<dependency_analysis_types::column> const& columns, uint32_t target_index, uint32_t other_index )
  {
    if ( columns[target_index].tt == columns[other_index].tt )
    {
      fmt::print( "EQUAL {} = {}\n", target_index, other_index );
    }
    else if ( columns[target_index].tt == ~columns[other_index].tt )
    {
      fmt::print( "EQUAL {} = ~{}\n", target_index, ~other_index );
    }
  }

  void check_nary_patterns( std::vector<dependency_analysis_types::column> const& columns, uint32_t target_index, std::vector<uint32_t> const& other_indices )
  {
    /* xor */
    if ( columns[target_index].tt == nary_xor( columns, other_indices ) )
    {
      fmt::print( "EQUAL {} = XOR ", target_index );
      for ( const auto& i : other_indices )
      {
        std::cout << i << ' ';
      }
      std::cout << std::endl;
    }
    if ( ~columns[target_index].tt == nary_xor( columns, other_indices ) )
    {
      fmt::print( "EQUAL {} = ~XOR ", target_index );
      for ( const auto& i : other_indices )
      {
        std::cout << i << ' ';
      }
      std::cout << std::endl;
    }

    for ( uint32_t polarity = 0u; polarity < ( 1u << other_indices.size() ); ++polarity )
    {
      /* convert polarity to complement flags */
      std::vector<bool> complement;
      auto copy_polarity = polarity;
      for ( auto i = 0u; i < other_indices.size(); ++i )
      {
        complement.push_back( ( copy_polarity & 1u ) );
        copy_polarity >>= 1u;
      }

      /* and */
      if ( columns[target_index].tt == nary_and( columns, other_indices, complement ) )
      {
        fmt::print( "EQUAL {} = AND ", target_index );
        for ( auto i = 0u; i < other_indices.size(); ++i )
        {
          if ( complement[i] )
            std::cout << "~";
          std::cout << other_indices[i] << ' ';
        }
        std::cout << std::endl;
      }
      if ( columns[target_index].tt == ~nary_and( columns, other_indices, complement ) )
      {
        fmt::print( "EQUAL {} = NAND ", target_index );
        for ( auto i = 0u; i < other_indices.size(); ++i )
        {
          if ( complement[i] )
            std::cout << "~";
          std::cout << other_indices[i] << ' ';
        }
        std::cout << std::endl;
      }
    }
  }

  kitty::partial_truth_table nary_and( std::vector<dependency_analysis_types::column> const& columns, std::vector<uint32_t> const& other_indices, std::vector<bool> const& complement )
  {
    /* compute nary and */
    auto result = complement[0u] ? ~columns[other_indices[0u]].tt : columns[other_indices[0u]].tt;
    for ( auto i = 1u; i < other_indices.size(); ++i )
    {
      result &= complement[i] ? ~columns[other_indices[i]].tt : columns[other_indices[i]].tt;
    }
    return result;
  }

  kitty::partial_truth_table nary_xor( std::vector<dependency_analysis_types::column> const& columns, std::vector<uint32_t> const& other_indices )
  {
    /* compute nary and */
    auto result = columns[other_indices[0u]].tt;
    for ( auto i = 1u; i < other_indices.size(); ++i )
    {
      result ^= columns[other_indices[i]].tt;
    }
    return result;
  }

private:
  dependency_analysis_params const& ps;
  dependency_analysis_stats &st;
}; /* dependency_analysis_impl */

dependency_analysis_result_type compute_dependencies( kitty::dynamic_truth_table const &tt )
{
  dependency_analysis_params ps;
  dependency_analysis_stats st;
  dependency_analysis_impl da( ps, st );

  auto const result = da.run( tt );

  st.report();

  return result;
}

} /* namespace angel */
