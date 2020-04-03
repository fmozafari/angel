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

    dependency_analysis2_result_type result;
    return result;
  }

  dependency_analysis2_params const& ps;
  dependency_analysis2_stats& st;
};

} /* namespace angel */
