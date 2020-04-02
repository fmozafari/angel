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
    XNOR  = 3,
    AND   = 4,
    NAND  = 5,
  };

  using fanins = std::vector<uint32_t>;
  using pattern = std::pair<pattern_kind, fanins>;

  /* column */
  struct column
  {
    kitty::partial_truth_table tt;
    uint32_t index;
  };

  static std::string pattern_kind_string( pattern_kind const kind )
  {
    switch( kind )
    {
    case pattern_kind::EQUAL:
      return "EQUAL";
    case pattern_kind::XOR:
      return "XOR";
    case pattern_kind::XNOR:
      return "~XOR";
    case pattern_kind::AND:
      return "AND";
    case pattern_kind::NAND:
      return "~AND";
    default:
      std::abort();
    }
  }

  static std::string pattern_string( pattern const& p )
  {
    std::string args;
    for ( const auto& a : p.second )
    {
      args += fmt::format( "{} ", a );
    }
    return fmt::format( "{}( {})", pattern_kind_string( p.first ), args );
  }
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

    dependency_analysis_result_type result;
    for ( auto i = 0u; i < num_vars; ++i )
    {
      patterns.clear();

      /* collect patterns */
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

      /* evaluate patterns */
      std::sort( std::begin( patterns ), std::end( patterns ),
                 [&]( const auto& a, const auto& b ){
                   auto const cost_a = cost( a );
                   auto const cost_b = cost( b );

                   /* compare CNOTs */
                   if ( cost_a.first < cost_b.first )
                   {
                     return true;
                   }

                   /* compare NOTs */
                   if ( cost_a.second < cost_b.second )
                   {
                     return true;
                   }

                   /* when costs are equal, compare structurally to ensure a total order */
                   if ( a.first < b.first )
                   {
                     return true;
                   }

                   if ( a.second.size() < b.second.size() )
                   {
                     return true;
                   }
                   else if ( a.second.size() == b.second.size() )
                   {
                     for ( auto i = 0u; i< a.second.size(); ++i )
                     {
                       if ( a.second[i] < b.second[i] )
                       {
                         return true;
                       }
                     }
                   }

                   return false;
                 } );

      // for ( const auto& p : patterns )
      // {
      //   std::cout << dependency_analysis_types::pattern_string( p ) << ' ' << cost( p ).first << ' ' << cost( p ).second << std::endl;
      // }

      if ( patterns.size() > 0u )
      {
        /* store the best dependency pattern */
        result.dependencies[i] = patterns[0u];
      }
    }

    /* TODO */
    return result;
  }

private:
  std::pair<uint32_t, uint32_t> cost( dependency_analysis_types::pattern const& p ) const
  {
    assert( p.second.size() > 0u );
    switch ( p.first )
    {
    case dependency_analysis_types::pattern_kind::EQUAL:
      {
        assert( p.second.size() == 1u );
        return { 1u, p.second[0] % 2u } ;
      }
    case dependency_analysis_types::pattern_kind::XOR:
      {
        return { p.second.size(), 0u };
      }
    case dependency_analysis_types::pattern_kind::XNOR:
      {
        return { p.second.size(), 1u };
      }
    case dependency_analysis_types::pattern_kind::AND:
      {
        auto const n = p.second.size();
        auto polarity_counter = 0u;
        for ( auto i = 0u; i < n; ++i )
        {
          polarity_counter += p.second[i] % 2;
        }
        return { 1u << ( n + 1 ) - 2u, polarity_counter };
      }
    case dependency_analysis_types::pattern_kind::NAND:
      {
        auto const n = p.second.size();
        auto polarity_counter = 1u;
        for ( auto i = 0u; i < n; ++i )
        {
          polarity_counter += p.second[i] % 2;
        }
        return { 1u << ( n + 1 ) - 2u, polarity_counter };
      }
    default:
      std::abort();
    }
  }

  bool check_unary_patterns( std::vector<dependency_analysis_types::column> const& columns, uint32_t target_index, uint32_t other_index )
  {
    bool found = false;
    if ( columns[target_index].tt == columns[other_index].tt )
    {
      patterns.emplace_back( dependency_analysis_types::pattern_kind::EQUAL, std::vector<uint32_t>{2u*other_index} );
      found = true;
    }
    else if ( columns[target_index].tt == ~columns[other_index].tt )
    {
      patterns.emplace_back( dependency_analysis_types::pattern_kind::EQUAL, std::vector<uint32_t>{2u*other_index + 1u} );
      found = true;
    }
    return found;
  }

  bool check_nary_patterns( std::vector<dependency_analysis_types::column> const& columns, uint32_t target_index, std::vector<uint32_t> const& other_indices )
  {
    bool found = false;

    /* xor */
    if ( columns[target_index].tt == nary_xor( columns, other_indices ) )
    {
      std::vector<uint32_t> fanins( other_indices.size() );
      for ( auto i = 0u; i < other_indices.size(); ++i )
      {
        fanins[i] = 2u*other_indices[i];
      }
      patterns.emplace_back( dependency_analysis_types::pattern_kind::XOR, fanins );
      found = true;
    }
    if ( ~columns[target_index].tt == nary_xor( columns, other_indices ) )
    {
      std::vector<uint32_t> fanins( other_indices.size() );
      for ( auto i = 0u; i < other_indices.size(); ++i )
      {
        fanins[i] = 2u*other_indices[i];
      }
      patterns.emplace_back( dependency_analysis_types::pattern_kind::XNOR, fanins );
      found = true;
    }

    /* and */
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

      if ( columns[target_index].tt == nary_and( columns, other_indices, complement ) )
      {
        std::vector<uint32_t> fanins( other_indices.size() );
        for ( auto i = 0u; i < other_indices.size(); ++i )
        {
          fanins[i] = 2u*other_indices[i] + complement[i];
        }
        patterns.emplace_back( dependency_analysis_types::pattern_kind::AND, fanins );
        found = true;
      }
      if ( columns[target_index].tt == ~nary_and( columns, other_indices, complement ) )
      {
        std::vector<uint32_t> fanins( other_indices.size() );
        for ( auto i = 0u; i < other_indices.size(); ++i )
        {
          fanins[i] = 2u*other_indices[i] + complement[i];
        }
        patterns.emplace_back( dependency_analysis_types::pattern_kind::NAND, fanins );
        found = true;
      }
    }
    return found;
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

  std::vector<dependency_analysis_types::pattern> patterns;
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
